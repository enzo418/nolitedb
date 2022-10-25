
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
#include "nldb/typedef.hpp"
#include "signal.h"

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

        auto expanded = repos->repositoryProperty->findAll(subColl->getId());

        Object composed(prop, {}, subColl->getId());

        // move expanded props into the composed property
        auto& props = composed.getPropertiesRef();
        props.insert(props.begin(), std::make_move_iterator(expanded.begin()),
                     std::make_move_iterator(expanded.end()));

        return std::move(composed);
    }

    void expandObjectProperties(Object& composed, auto&,
                                std::shared_ptr<Repositories> const& repos);

    template <typename IT>
    requires std::output_iterator<IT, Object>
    void expandObjectProperties(const Property& prop, IT& it,
                                std::shared_ptr<Repositories> const& repos) {
        if (prop.getType() == PropertyType::OBJECT) {
            try {
                auto subColl =
                    repos->repositoryCollection->findByOwner(prop.getId());

                if (!subColl) {
                    // return Object::empty();
                    throw std::runtime_error(
                        "Couldn't expand property with id " +
                        std::to_string(prop.getId()));
                }

                Object composed(prop, {}, subColl->getId());

                expandObjectProperties(composed, it, repos);

                // change the property of type object to a composed property
                *it = composed;
            } catch (...) {
            }
        }
    }

    void expandObjectProperties(const AggregatedProperty& prop, auto&,
                                std::shared_ptr<Repositories> const& repos) {}

    void expandObjectProperties(Object& composed, auto&,
                                std::shared_ptr<Repositories> const& repos) {
        auto& props = composed.getPropertiesRef();

        if (props.empty()) {
            auto expanded =
                repos->repositoryProperty->findAll(composed.getCollId());

            props.insert(props.begin(),
                         std::make_move_iterator(expanded.begin()),
                         std::make_move_iterator(expanded.end()));
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
                                std::forward_list<SelectableProperty>& select) {
        for (auto it = select.begin(); it != select.end(); it++) {
            std::visit(
                [&it, repos](auto& prop) {
                    expandObjectProperties(prop, it, repos);
                },
                *it);
        }
    }

    /* -------------------- SELECT CLAUSE ------------------- */
    void addSelectClause(std::stringstream& sql, const Property& prop,
                         QueryRunnerCtx& ctx, snowflake currentSelectCollId) {
        // If the condition is false, then we already have its value and
        // therefore we use its alias {prop.name}_{id}, otherwise it writes a
        // statement to get its value.
        if (prop.getCollectionId() == currentSelectCollId) {
            sql << ctx.getValueExpression(prop) + " as " +
                       std::string(ctx.getAlias(prop));
        } else {
            sql << ctx.getAlias(prop);
        }
    }

    void addSelectClause(std::stringstream& sql,
                         const AggregatedProperty& agProp, QueryRunnerCtx& ctx,
                         snowflake currentSelectCollId) {
        const auto& prop = agProp.property;
        const auto collID = prop.getCollectionId();

        if (currentSelectCollId == ctx.getRootCollId()) {
            // we were requested by the first select statement, we need to do
            // the aggregation.

            std::string alias = collID == currentSelectCollId
                                    ? ctx.getValueExpression(prop)
                                    : std::string(ctx.getAlias(prop));
            sql << parseSQL(
                "@ag_type(@prop_alias) as @alias",
                {{"@ag_type", std::string(magic_enum::enum_name(agProp.type))},
                 {"@prop_alias", alias},
                 {"@alias", ctx.getAlias(agProp)}},
                false);

        } else if (collID == currentSelectCollId) {
            // is not the root select, we need to get its value
            sql << ctx.getValueExpression(prop) + " as " +
                       std::string(ctx.getAlias(prop));
        } else {
            // is a pass through
            sql << ctx.getAlias(agProp);
        }
    }

    void addSelectClause(std::stringstream& sql, Object& composed,
                         QueryRunnerCtx& ctx, snowflake currentSelectCollId) {
        ctx.set(composed);

        auto& props = composed.getPropertiesRef();

        for (int i = 0; i < props.size(); i++) {
            // why don't we use the composed.collId?
            // because we aren't building the select for ourself, we are
            // building it for `currentSelectCollId`.
            std::visit(
                [&sql, &ctx, currentSelectCollId](auto& p) {
                    addSelectClause(sql, p, ctx, currentSelectCollId);
                },
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
            std::visit(
                [&sql, &ctx](auto& p) {
                    addSelectClause(sql, p, ctx, ctx.getRootCollId());
                },
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
                       std::vector<snowflake>& ids, QueryRunnerCtx& ctx,
                       std::string_view docAlias = doc_alias) {
        if (!IN_VEC(ids, prop.getId()) && prop.getType() != PropertyType::ID) {
            auto& tables = definitions::tables::getPropertyTypesTable();

            NLDB_ASSERT(prop.getType() != PropertyType::OBJECT,
                        "Property should be a value");

            sql << parseSQL(
                " left join @table as @row_alias on (@row_alias.prop_id = "
                "@prop_id and @doc_alias.id = @row_alias.obj_id)\n",
                {{"@table", tables[prop.getType()]},
                 {"@row_alias", ctx.getAlias(prop)},
                 {"@prop_id", prop.getId()},
                 {"@doc_alias", docAlias}},
                false);

            ids.push_back(prop.getId());
        }
    }

    void addFromClause(std::stringstream& sql, Object& composed,
                       std::vector<snowflake>& ids, QueryRunnerCtx& ctx,
                       std::string_view docAlias = doc_alias) {
        ctx.set(composed);
        auto prop = composed.getProperty();
        std::string_view alias = ctx.getAlias(prop);
        auto& props = composed.getPropertiesRef();
        // addFromClause(sql, composed.getProperty(), ids, ctx, docAlias);

        sql << "left join (select ";

        addSelectClause(sql, composed, ctx, composed.getCollId());

        // select obj_id
        sql << parseSQL(" @comma @alias.obj_id as 'obj_id' ",
                        {{"@comma", std::string(props.empty() ? "" : ",")},
                         {"@alias", alias}},
                        false);

        // add main from
        sql << parseSQL(" from 'object' as @alias ", {{"@alias", alias}},
                        false);

        auto composedCB = overloaded {
            [&sql, &ctx, &ids, &composed, &alias](const Property& prop) {
                addFromClause(sql, prop, ids, ctx, alias);
            },
            [&sql, &ctx, &ids, &composed, &alias](Object& composed2) {
                addFromClause(sql, composed2, ids, ctx, alias);
            }};

        for (auto& p : props) {
            std::visit(composedCB, p);
        }

        sql << parseSQL(" where @alias.prop_id = @prop_id ",
                        {{"@alias", alias}, {"@prop_id", prop.getId()}}, false);

        // consume obj id
        if (prop.getCollectionId() == NullID) {
            // it's a root collection, it doesn't depend on this object.
            sql << parseSQL(" ) as @obj_alias", {{"@obj_alias", alias}});
        } else {
            sql << parseSQL(
                " ) as @obj_alias on @doc_alias.id = @obj_alias.obj_id ",
                {{"@obj_alias", alias}, {"@doc_alias", docAlias}}, false);
        }
    }

    void addFromClause(std::stringstream& sql,
                       std::forward_list<SelectableProperty>& props,
                       std::vector<snowflake>& ids, QueryRunnerCtx& ctx) {
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
                       std::vector<snowflake>& ids, QueryRunnerCtx& ctx) {
        for (auto& p : props) {
            addFromClause(sql, p, ids, ctx);
        }
    }

    void addFromClause(std::stringstream& sql,
                       std::vector<SortedProperty>& props,
                       std::vector<snowflake>& ids, QueryRunnerCtx& ctx) {
        for (auto& p : props) {
            addFromClause(sql, p.property, ids, ctx);
        }
    }

    void addFromClause(std::stringstream& sql,
                       PropertyExpressionOperand const& prop,
                       std::vector<snowflake>& ids, QueryRunnerCtx& ctx) {
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
                       std::vector<snowflake>& ids, QueryRunnerCtx& ctx) {
        addFromClause(sql, props.left, ids, ctx);
        addFromClause(sql, props.right, ids, ctx);
    }

    void addFromClause(std::stringstream& sql, QueryPlannerContextSelect& data,
                       QueryRunnerCtx& ctx) {
        // main from is document/object
        sql << " from 'object' " << doc_alias << "\n";

        // add all properties that appear in the data
        std::vector<snowflake> ids;
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
                sql << ctx.getContextualizedAlias(prop, ctx.getRootCollId());
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
            sql << ctx.getAlias(props[i]);

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
            sql << ctx.getAlias(props[i].property) << " "
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

    void selectAllOnEmpty(QueryPlannerContextSelect& data,
                          std::shared_ptr<Repositories> const& repos) {
        if (data.select_value.empty()) {
            auto allProps =
                repos->repositoryProperty->findAll(data.from.begin()->getId());

            data.select_value.assign(std::make_move_iterator(allProps.begin()),
                                     std::make_move_iterator(allProps.end()));
        }
    }

    /* ------------------------ READ ------------------------ */
    void read(const Property& prop, std::shared_ptr<IDBRowReader> row, int& i,
              json& out) {
        if (!row->isNull(i)) {
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
                    out["_id"] = row->readInt64(i);
                    break;
                case PropertyType::ARRAY:
                    out[prop.getName()] = json::parse(row->readString(i));
                    break;
                case PropertyType::BOOLEAN:
                    out[prop.getName()] = row->readInt32(i) == 1 ? true : false;
                    break;
                default:
                    throw std::runtime_error(
                        "Select properties should not hold a reserved "
                        "property directly!");
                    break;
            }
        }

        i++;
    }

    void read(const AggregatedProperty& prop, std::shared_ptr<IDBRowReader> row,
              int& i, json& out) {
        out[prop.alias] = row->readInt64(i);
        i++;
    }

    void read(Object& composed, std::shared_ptr<IDBRowReader> row, int& i,
              json& out) {
        const std::string& name = composed.getProperty().getName();
        // out[name] = json::object();  // an object

        json temp;

        auto& propsRef = composed.getPropertiesRef();
        for (auto& prop : propsRef) {
            std::visit([&row, &i, &temp](auto& t) { read(t, row, i, temp); },
                       prop);
        }

        if (!temp.is_null()) out[name] = std::move(temp);
    }

    void printSelect(Property& p, int tab = 0) {
        std::cout << std::string(tab, ' ') << p.getName() << ": " << p.getId()
                  << std::endl;
    }

    void printSelect(AggregatedProperty& p, int tab = 0) {
        std::cout << std::string(tab, ' ') << "["
                  << std::string(magic_enum::enum_name(p.type)) << "]"
                  << p.property.getName() << ": " << p.property.getId()
                  << std::endl;
    }

    void printSelect(Object& p, int tab = 0) {
        std::cout << std::string(tab, ' ') << "- " << p.getProperty().getName()
                  << std::endl;

        auto cb = [&tab](auto& p) { printSelect(p, tab + 2); };

        for (auto& prop : p.getPropertiesRef()) {
            std::visit(cb, prop);
        }
    }

    void printSelect(std::forward_list<SelectableProperty>& list) {
        auto cb = [](auto& p) { printSelect(p, 2); };

        std::cout << "\nSelect:\n";

        for (auto& st : list) {
            std::visit(cb, st);
        }

        std::cout << "\n";
    }

    /* ------------------- EXECUTE SELECT ------------------- */
    json QueryRunnerSQ3::select(QueryPlannerContextSelect&& data) {
        populateData(data);

        selectAllOnEmpty(data, repos);

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

#ifdef NLDB_DEBUG_QUERY
        printSelect(data.select_value);
#endif

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
            }

            if (!rowValue.is_null()) result.push_back(std::move(rowValue));
        }

        // read output and build object
        return result;
    }
}  // namespace nldb