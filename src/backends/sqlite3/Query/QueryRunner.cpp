#include "QueryRunner.hpp"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>

#include "backends/sqlite3/DAL/Definitions.hpp"
#include "magic_enum.hpp"
#include "nldb/Collection.hpp"
#include "nldb/DAL/Repositories.hpp"
#include "nldb/Exceptions.hpp"
#include "nldb/LOG/log.hpp"
#include "nldb/Object.hpp"
#include "nldb/Property/AggregatedProperty.hpp"
#include "nldb/Property/Property.hpp"
#include "nldb/Property/PropertyExpression.hpp"
#include "nldb/Property/SortedProperty.hpp"
#include "nldb/Query/QueryContext.hpp"
#include "nldb/Utils/Enums.hpp"
#include "nldb/Utils/ParamsBindHelpers.hpp"
#include "nldb/Utils/Variant.hpp"
#include "nldb/nldb_json.hpp"

#define IN_VEC(vec, x) (std::find(vec.begin(), vec.end(), x) != vec.end())

namespace nldb {
    QueryRunnerSQ3::QueryRunnerSQ3(IDB* pConnection,
                                   std::shared_ptr<Repositories> repos)
        : QueryRunner(pConnection, repos) {}

    using namespace ::utils::paramsbind;

    const std::string doc_alias = "__doc";

    /* -------------- FILTER OUT EMPTY OBJECTS -------------- */
    void filterOutEmptyObjects(std::forward_list<SelectableProperty>& data) {
        auto cb = overloaded {[](Object& composed) {
                                  return composed.getPropertiesRef().empty();
                              },
                              [](const Property& prop) {
                                  // something went wrong at the expansion,
                                  // delete it
                                  return prop.getType() == OBJECT;
                              },
                              [](const auto&) { return false; }};

        for (auto prev_it = data.before_begin(); prev_it != data.end();
             prev_it++) {
            auto it = std::next(prev_it);
            if (it != data.end()) {
                if (std::visit(cb, *it)) {
                    data.erase_after(prev_it);
                }
            }
        }
    }

    /* -------------- EXPAND OBJECT PROPERTIES -------------- */
    /**
     * Property of type object and ComposedProperty with 0 sub-items gets
     * expanded into all its properties.
     *
     * If a collection has {name, contact: {address, email}} and
     * we select(name, contact) then we expect the resulting json
     * to be {name: "..", contact: {address: "..", email: ".."}},
     * but as properties in the query planner represents a row in
     * the property table, this wouldn't happen.
     *
     * The objective of this function is to convert each property
     * of type object its sub-properties and leave some record of
     * that so when we read the resulting query we know what to
     * expect.
     *
     * I will choose the approach where we get the collection from
     * the value_object table. This have the main drawback that if
     * a sub-collection doesn't have at least one document, we will
     * not be able to expand it, which i think is ok since that
     * is just like an empty json object.
     *
     * A different way of doing it would be creating a new table
     * and relate a property with its sub-collection.
     */

    /**
     * @brief Converts a property of type object to a composed property (object)
     * throws if it couldn't find the collection
     */
    inline Object ObjectPropertyToComposed(
        const Property& prop, std::shared_ptr<Repositories> const& repos) {
        NLDB_ASSERT(prop.getType() == PropertyType::OBJECT,
                    "Function misuse, expected a property of type object");

        auto subColl = repos->repositoryCollection->findByOwner(prop.getId());

        if (!subColl) {
            // return Object::empty();
            throw std::runtime_error("Couldn't expand property with id " +
                                     std::to_string(prop.getId()));
        }

        auto expanded = repos->repositoryProperty->find(subColl->getId());

        Object composed(prop, {}, subColl->getId());

        // move expanded props into the composed property
        auto& props = composed.getPropertiesRef();
        props.insert(props.begin(), std::make_move_iterator(expanded.begin()),
                     std::make_move_iterator(expanded.end()));

        return std::move(composed);
    }

    template <typename IT>
    requires std::output_iterator<IT, Object>
    void expandObjectProperties(const Property& prop, IT& it,
                                std::shared_ptr<Repositories> const& repos) {
        if (prop.getType() == PropertyType::OBJECT) {
            try {
                Object composed = ObjectPropertyToComposed(prop, repos);
                // change the property of type object to a composed property
                *it = composed;
            } catch (...) {
            }
        }
    }

    void expandObjectProperties(const AggregatedProperty& prop, auto& it,
                                std::shared_ptr<Repositories> const& repos) {}

    void expandObjectProperties(Object& composed, auto& it_parent,
                                std::shared_ptr<Repositories> const& repos) {
        auto& props = composed.getPropertiesRef();

        if (props.empty()) {
            auto p = composed.getProperty();

            auto coll = repos->repositoryCollection->findByOwner(p.getId());

            if (!coll) return;  // leave it empty, we will remove it later

            auto expanded = repos->repositoryProperty->find(coll->getId());

            props.insert(props.begin(), expanded.begin(), expanded.end());
        }

        for (auto it = props.begin(); it != props.end(); it++) {
            std::visit(
                [&it, repos](auto& prop) {
                    expandObjectProperties(prop, it, repos);
                },
                *it);
        }
    }

    void expandObjectProperties(std::shared_ptr<Repositories> const& repos,
                                std::forward_list<SelectableProperty>& data) {
        for (auto it = data.begin(); it != data.end(); it++) {
            std::visit(
                [&data, &it, repos](auto& prop) {
                    expandObjectProperties(prop, it, repos);
                },
                *it);
        }
    }

    /* -------------------- SELECT CLAUSE ------------------- */
    void addSelectClause(std::stringstream& sql, const Property& prop,
                         QueryRunnerCtx& ctx) {
        sql << ctx.getValueExpression(prop) << " as " << ctx.getAlias(prop);
        std::cout << sql.str() << "\n\n";
    }

    void addSelectClause(std::stringstream& sql,
                         const AggregatedProperty& agProp,
                         QueryRunnerCtx& ctx) {
        sql << parseSQL(
            "@ag_type(@ag_prop) as @alias",
            {{"@ag_type", std::string(magic_enum::enum_name(agProp.type))},
             {"@ag_prop", ctx.getValueExpression(agProp.property)},
             {"@alias", ctx.getAlias(agProp)}},
            false);
    }

    void addSelectClause(std::stringstream& sql, Object& composed,
                         QueryRunnerCtx& ctx) {
        ctx.set(composed);

        auto& props = composed.getPropertiesRef();

        for (int i = 0; i < props.size(); i++) {
            std::visit([&sql, &ctx](auto& p) { addSelectClause(sql, p, ctx); },
                       props[i]);

            if (i != props.size() - 1) {
                sql << ", ";
            }
        }
    }

    void addSelectClause(std::stringstream& sql,
                         std::forward_list<SelectableProperty>& props,
                         QueryRunnerCtx& ctx) {
        sql << "select ";

        for (auto it = props.begin(); it != props.end(); it++) {
            std::visit([&sql, &ctx](auto& p) { addSelectClause(sql, p, ctx); },
                       *it);

            if (std::next(it) != props.end()) {
                sql << ", ";
            }
        }
    }

    /* ---------------- FROM/JOIN CLAUSE ---------------- */
    /**
     * @brief This is where all the addFrom callbacks converge.
     *
     * @param sql sql query
     * @param prop property
     * @param ids list of ids used.
     */
    void addFromClause(std::stringstream& sql, const Property& prop,
                       std::vector<int>& ids, QueryRunnerCtx& ctx,
                       std::string_view docAlias = doc_alias) {
        if (!IN_VEC(ids, prop.getId()) && prop.getType() != PropertyType::ID) {
            auto& tables = definitions::tables::getPropertyTypesTable();

            sql << parseSQL(
                " left join @table as @row_alias on (@doc_alias.id = "
                "@row_alias.obj_id and @row_alias.prop_id = @prop_id)\n",
                {{"@table", tables[prop.getType()]},
                 {"@row_alias", ctx.getAlias(prop)},
                 {"@prop_id", prop.getId()},
                 {"@doc_alias", docAlias}},
                false);

            ids.push_back(prop.getId());
        }
    }

    void addFromClause(std::stringstream& sql, Object& composed,
                       std::vector<int>& ids, QueryRunnerCtx& ctx,
                       std::string_view docAlias = doc_alias) {
        ctx.set(composed);
        addFromClause(sql, composed.getProperty(), ids, ctx, docAlias);

        auto& props = composed.getPropertiesRef();

        auto composedCB =
            overloaded {[&sql, &ctx, &ids, &composed](const Property& prop) {
                            addFromClause(sql, prop, ids, ctx,
                                          ctx.getAlias(composed.getProperty()));
                        },
                        [&sql, &ctx, &ids, &composed](Object& composed2) {
                            addFromClause(sql, composed2, ids, ctx,
                                          ctx.getAlias(composed.getProperty()));
                        }};

        for (auto& p : props) {
            std::visit(composedCB, p);
        }
    }

    void addFromClause(std::stringstream& sql,
                       std::forward_list<SelectableProperty>& props,
                       std::vector<int>& ids, QueryRunnerCtx& ctx) {
        auto cb = overloaded {
            [&sql, &ctx, &ids](const Property& prop) {
                addFromClause(sql, prop, ids, ctx);
            },
            [&sql, &ctx, &ids](const AggregatedProperty& agProp) {
                addFromClause(sql, agProp.property, ids, ctx);
            },
            [&sql, &ctx, &ids](Object& composed) {
                addFromClause(sql, composed, ids, ctx);
            },
        };

        for (auto& p : props) {
            std::visit(cb, p);
        }
    }

    void addFromClause(std::stringstream& sql, std::vector<Property>& props,
                       std::vector<int>& ids, QueryRunnerCtx& ctx) {
        for (auto& p : props) {
            addFromClause(sql, p, ids, ctx);
        }
    }

    void addFromClause(std::stringstream& sql,
                       std::vector<SortedProperty>& props,
                       std::vector<int>& ids, QueryRunnerCtx& ctx) {
        for (auto& p : props) {
            addFromClause(sql, p.property, ids, ctx);
        }
    }

    void addFromClause(std::stringstream& sql,
                       PropertyExpressionOperand const& prop,
                       std::vector<int>& ids, QueryRunnerCtx& ctx) {
        auto cb = overloaded {
            [&sql, &ctx, &ids](LogicConstValue const& prop) {
                if (std::holds_alternative<Property>(prop)) {
                    addFromClause(sql, std::get<Property>(prop), ids, ctx);
                }
            },
            [&sql, &ctx, &ids](box<struct PropertyExpression> const& agProp) {
                addFromClause(sql, agProp->left, ids, ctx);
                addFromClause(sql, agProp->right, ids, ctx);
            },
            [&sql, &ctx, &ids](PropertyExpressionOperand const& agProp) {
                addFromClause(sql, agProp, ids, ctx);
            }};

        std::visit(cb, prop);
    }

    void addFromClause(std::stringstream& sql, PropertyExpression& props,
                       std::vector<int>& ids, QueryRunnerCtx& ctx) {
        addFromClause(sql, props.left, ids, ctx);
        addFromClause(sql, props.right, ids, ctx);
    }

    void addFromClause(std::stringstream& sql, QueryPlannerContextSelect& data,
                       QueryRunnerCtx& ctx) {
        // main from is document/object
        sql << " from 'object' " << doc_alias << "\n";

        // add all properties that appear in the data
        std::vector<int> ids;
        addFromClause(sql, data.select_value, ids, ctx);
        addFromClause(sql, data.groupBy_value, ids, ctx);
        addFromClause(sql, data.sortBy_value, ids, ctx);

        if (data.where_value)
            addFromClause(sql, data.where_value.value(), ids, ctx);
    }

    /* ------------------ WHERE CLAUSE ------------------ */
    void addWhereClause(std::stringstream& sql, PropertyExpression const& expr,
                        QueryRunnerCtx& ctx);

    void addWhereExpression(std::stringstream& sql,
                            PropertyExpressionOperand const& expr,
                            QueryRunnerCtx& ctx) {
        auto cbConstVal = overloaded {
            [&sql, &ctx](const Property& prop) {
                sql << ctx.getValueExpression(prop);
            },
            [&sql, &ctx](const std::string& str) {
                sql << encloseQuotesConst(str);
            },
            [&sql, &ctx](int val) { sql << val; },
            [&sql, &ctx](double val) { sql << val; },
            [&sql, &ctx](const char* str) { sql << encloseQuotesConst(str); }};

        auto cbOperand = overloaded {
            [&sql, &ctx, &cbConstVal](LogicConstValue const& prop) {
                std::visit(cbConstVal, prop);
            },
            [&sql, &ctx](box<struct PropertyExpression> const& agProp) {
                addWhereClause(sql, *agProp, ctx);
            },
            [&sql, &ctx](PropertyExpressionOperand const& agProp) {
                addWhereExpression(sql, agProp, ctx);
            }};

        std::visit(cbOperand, expr);
    }

    void addWhereClause(std::stringstream& sql, PropertyExpression const& expr,
                        QueryRunnerCtx& ctx) {
        if (expr.type != PropertyExpressionOperator::NOT) {
            addWhereExpression(sql, expr.left, ctx);
            sql << " " << utils::OperatorToString(expr.type) << " ";
            addWhereExpression(sql, expr.right, ctx);
        } else {
            sql << " " << utils::OperatorToString(expr.type) << " ";
            addWhereExpression(sql, expr.left, ctx);
        }
    }

    void addWhereClause(std::stringstream& sql, QueryPlannerContextSelect& data,
                        QueryRunnerCtx& ctx) {
        sql << " WHERE "
            << parseSQL("@doc.prop_id = @root_prop_id",
                        {{"@doc", std::string(doc_alias)},
                         {"@root_prop_id", ctx.getRootPropId()}},
                        false);

        if (data.where_value) {
            sql << " AND ";

            addWhereExpression(sql, data.where_value.value(), ctx);
        }
    }

    /* ----------------- GROUP BY CLAUSE ---------------- */
    void addGroupByClause(std::stringstream& sql, std::vector<Property>& props,
                          QueryRunnerCtx& ctx) {
        if (props.empty()) return;

        sql << " GROUP BY ";

        for (int i = 0; i < props.size(); i++) {
            sql << ctx.getValueExpression(props[i]);

            if (i != props.size() - 1) {
                sql << ", ";
            }
        }
    }

    /* --------------------- SORT CLAUSE -------------------- */
    void addOrderByClause(std::stringstream& sql,
                          std::vector<SortedProperty>& props,
                          QueryRunnerCtx& ctx) {
        if (props.empty()) return;

        sql << " ORDER BY ";

        for (int i = 0; i < props.size(); i++) {
            sql << ctx.getValueExpression(props[i].property) << " "
                << magic_enum::enum_name(props[i].type);

            if (i != props.size() - 1) {
                sql << ", ";
            }
        }
    }

    /* ------------------ LIMIT CLAUSE ------------------ */
    void addPaginationClause(std::stringstream& sql,
                             const QueryPagination& limit) {
        sql << " limit " << limit.elementsPerPage << " offset "
            << (limit.pageNumber - 1) * limit.elementsPerPage;
    }

    /* ------------------------ READ ------------------------ */
    void read(const Property& prop, std::shared_ptr<IDBRowReader> row, int& i,
              json& out) {
        if (row->isNull(i)) {
            // Should we set the field to null?
            // jrow[prop.getName()] = nullptr;
            return;
        }

        switch (prop.getType()) {
            case PropertyType::INTEGER:
                out[prop.getName()] = row->readInt64(i);
                break;
            case PropertyType::DOUBLE:
                out[prop.getName()] = row->readDouble(i);
                break;
            case PropertyType::STRING:
                out[prop.getName()] = row->readString(i);
                break;
            case PropertyType::ID:
                out["id"] = row->readInt32(i);
                break;
            case PropertyType::ARRAY:
                out[prop.getName()] = json::parse(row->readString(i));
                break;
            default:
                throw std::runtime_error(
                    "Select properties should not hold a reserved "
                    "property directly!");
                break;
        }
    }

    void read(const AggregatedProperty& prop, std::shared_ptr<IDBRowReader> row,
              int& i, json& out) {
        out[prop.alias] = row->readInt64(i);
    }

    void read(Object& composed, std::shared_ptr<IDBRowReader> row, int& i,
              json& out) {
        const std::string& name = composed.getProperty().getName();
        // out[name] = json::object();  // an object

        json temp;

        auto& propsRef = composed.getPropertiesRef();
        for (auto& prop : propsRef) {
            if (std::holds_alternative<Property>(prop)) {
                read(std::get<Property>(prop), row, i, temp);
                i++;
            } else {
                read(std::get<Object>(prop), row, i, temp);
            }
        }

        if (!temp.is_null()) out[name] = std::move(temp);
    }

    /* ------------------- EXECUTE SELECT ------------------- */
    json QueryRunnerSQ3::select(QueryPlannerContextSelect&& data) {
        populateData(data);

        std::stringstream sql;

        expandObjectProperties(repos, data.select_value);
        filterOutEmptyObjects(data.select_value);

        auto rootColl =
            repos->repositoryCollection->find(data.from.begin()->getName());

        if (!rootColl) {
            return {};  // the collection doesn't even exists
        }

        QueryRunnerCtx ctx(
            rootColl->getId(),
            repos->repositoryCollection->getOwnerId(rootColl->getId())
                .value_or(-1),
            doc_alias);

        addSelectClause(sql, data.select_value, ctx);
        addFromClause(sql, data, ctx);
        addWhereClause(sql, data, ctx);
        addGroupByClause(sql, data.groupBy_value, ctx);
        addOrderByClause(sql, data.sortBy_value, ctx);
        if (data.pagination_value)
            addPaginationClause(sql, data.pagination_value.value());

        // execute it
        auto reader = this->connection->executeReader(sql.str(), {});
        std::shared_ptr<IDBRowReader> row;

        json result = json::array();
        auto begin = data.select_value.begin();
        auto end = data.select_value.end();

        while (reader->readRow(row)) {
            json rowValue;

            int i = 0;
            for (auto it = begin; it != end; it++) {
                std::visit([&row, &i, &rowValue](
                               auto& val) { read(val, row, i, rowValue); },
                           *it);
                i++;
            }

            if (!rowValue.is_null()) result.push_back(std::move(rowValue));
        }

        // read output and build object
        return result;
    }
}  // namespace nldb