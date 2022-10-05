#include "QueryRunner.hpp"

#include <algorithm>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>

#include "backends/sqlite3/DAL/Definitions.hpp"
#include "magic_enum.hpp"
#include "nldb/Collection.hpp"
#include "nldb/Exceptions.hpp"
#include "nldb/LOG/log.hpp"
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
    QueryRunnerSQ3::QueryRunnerSQ3(IDB* pConnection)
        : QueryRunner(pConnection) {}

    using namespace ::utils::paramsbind;

    const char* doc_alias = "__doc";

    /**
     * @brief Get an alias. Because a property can have the same name as other
     * property from another collection.
     */
    std::string getAlias(const Property& prop) {
        return prop.getName() + "_" + std::to_string(prop.getId());
    }

    std::string getAlias(const AggregatedProperty& agProp) {
        return "ag_" + std::string(magic_enum::enum_name(agProp.type)) + "_" +
               getAlias(agProp.property);
    }

    /* -------------- EXPAND OBJECT PROPERTIES -------------- */
    struct ExpandedProperty {
        const char* alias;
        std::vector<SelectableProperty> props;
    };

    /**
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
    void expandObjectProperties(QueryPlannerContextSelect& data) {
        // allocate a new vector, because we need to keep the order of the
        // given select.
        std::vector<SelectableProperty> expanded;
        expanded.reserve(data.select_value.size());  // min len = select len

        auto cb = overloaded {
            [&expanded](const Property& prop) {
                if (prop.getType() == PropertyType::OBJECT) {
                } else {
                    expanded.push_back(std::move(prop));
                }
            },
            [](const AggregatedProperty& prop) {
                // do nothing
            },
        };

        for (auto& prop : data.select_value) {
            std::visit(cb, prop);
        }

        data.select_value = std::move(expanded);
    }

    /* -------------------- SELECT CLAUSE ------------------- */
    void addSelectClause(std::stringstream& sql,
                         std::vector<SelectableProperty>& props) {
        sql << "select ";

        auto cb = overloaded {
            [&sql](const Property& prop) {
                sql << encloseQuotesConst(getAlias(prop));
            },
            [&sql](const AggregatedProperty& agProp) {
                sql << parseSQL(
                    "@ag_type(@ag_prop) as @alias",
                    {{"@ag_type",
                      std::string(magic_enum::enum_name(agProp.type))},
                     {"@ag_prop",
                      encloseQuotesConst(getAlias(agProp.property))},
                     {"@alias", encloseQuotesConst(getAlias(agProp))}},
                    false);
            },
        };

        for (int i = 0; i < props.size(); i++) {
            std::visit(cb, props[i]);

            if (i != props.size() - 1) {
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
                       std::vector<int>& ids) {
        if (!IN_VEC(ids, prop.getId())) {
            auto& tables = definitions::tables::getPropertyTypesTable();

            sql << parseSQL(
                " left join @table as @row_alias on (@doc_alias.id = "
                "@row_alias.doc_id and @row_alias.prop_id = @prop_id)\n",
                {{"@table", tables[prop.getType()]},
                 {"@row_alias", encloseQuotesConst(getAlias(prop))},
                 {"@prop_id", prop.getId()},
                 {"@doc_alias", doc_alias}},
                false);

            ids.push_back(prop.getId());
        }
    }

    void addFromClause(std::stringstream& sql,
                       std::vector<SelectableProperty>& props,
                       std::vector<int>& ids) {
        auto cb = overloaded {
            [&sql, &ids](const Property& prop) {
                addFromClause(sql, prop, ids);
            },
            [&sql, &ids](const AggregatedProperty& agProp) {
                addFromClause(sql, agProp.property, ids);
            },
        };

        for (auto& p : props) {
            std::visit(cb, p);
        }
    }

    void addFromClause(std::stringstream& sql, std::vector<Property>& props,
                       std::vector<int>& ids) {
        for (auto& p : props) {
            addFromClause(sql, p, ids);
        }
    }

    void addFromClause(std::stringstream& sql,
                       std::vector<SortedProperty>& props,
                       std::vector<int>& ids) {
        for (auto& p : props) {
            addFromClause(sql, p.property, ids);
        }
    }

    void addFromClause(std::stringstream& sql,
                       const PropertyExpressionOperand& prop,
                       std::vector<int>& ids) {
        auto cb = overloaded {
            [&sql, &ids](const LogicConstValue& prop) {
                if (std::holds_alternative<Property>(prop)) {
                    addFromClause(sql, prop, ids);
                }
            },
            [&sql, &ids](const box<struct PropertyExpression>& agProp) {
                addFromClause(sql, agProp->left, ids);
                addFromClause(sql, agProp->right, ids);
            },
            [&sql, &ids](const PropertyExpressionOperand& agProp) {
                addFromClause(sql, agProp, ids);
            }};

        std::visit(cb, prop);
    }

    void addFromClause(std::stringstream& sql, const PropertyExpression& props,
                       std::vector<int>& ids) {
        addFromClause(sql, props.left, ids);
        addFromClause(sql, props.right, ids);
    }

    void addFromClause(std::stringstream& sql,
                       QueryPlannerContextSelect& data) {
        // main from is document
        sql << " from 'document' " << doc_alias << "\n";

        // add all properties that appear in the data
        std::vector<int> ids;
        addFromClause(sql, data.select_value, ids);
        addFromClause(sql, data.groupBy_value, ids);
        addFromClause(sql, data.sortBy_value, ids);

        if (data.where_value) addFromClause(sql, data.where_value.value(), ids);
    }

    /* ------------------ WHERE CLAUSE ------------------ */
    void addWhereClause(std::stringstream& sql, const PropertyExpression& expr);

    void addWhereExpression(std::stringstream& sql,
                            const PropertyExpressionOperand& expr) {
        auto cbConstVal = overloaded {
            [&sql](const Property& prop) {
                sql << encloseQuotesConst(getAlias(prop));
            },
            [&sql](const std::string& str) { sql << encloseQuotesConst(str); },
            [&sql](int val) { sql << val; }, [&sql](double val) { sql << val; },
            [&sql](const char* str) { sql << encloseQuotesConst(str); }};

        auto cbOperand =
            overloaded {[&sql, &cbConstVal](const LogicConstValue& prop) {
                            std::visit(cbConstVal, prop);
                        },
                        [&sql](const box<struct PropertyExpression>& agProp) {
                            addWhereClause(sql, *agProp);
                        },
                        [&sql](const PropertyExpressionOperand& agProp) {
                            addWhereExpression(sql, agProp);
                        }};
    }

    void addWhereClause(std::stringstream& sql,
                        const PropertyExpression& expr) {
        if (expr.type != PropertyExpressionOperator::NOT) {
            addWhereExpression(sql, expr.left);
            sql << " " << utils::OperatorToString(expr.type) << " ";
            addWhereExpression(sql, expr.right);
        } else {
            sql << " " << utils::OperatorToString(expr.type) << " ";
            addWhereExpression(sql, expr.left);
        }
    }

    void addWhereClause(std::stringstream& sql,
                        const QueryPlannerContextSelect& data) {
        if (data.where_value) {
            sql << " WHERE ";

            addWhereExpression(sql, data.where_value.value());
        }
    }

    /* ----------------- GROUP BY CLAUSE ---------------- */
    void addGroupByClause(std::stringstream& sql,
                          std::vector<Property>& props) {
        if (props.empty()) return;

        sql << " GROUP BY ";

        for (int i = 0; i < props.size(); i++) {
            sql << encloseQuotesConst(getAlias(props[i]));

            if (i != props.size() - 1) {
                sql << ", ";
            }
        }
    }

    /* --------------------- SORT CLAUSE -------------------- */
    void addOrderByClause(std::stringstream& sql,
                          std::vector<SortedProperty>& props) {
        if (props.empty()) return;

        sql << " ORDER BY ";

        for (int i = 0; i < props.size(); i++) {
            sql << encloseQuotesConst(getAlias(props[i].property)) << " "
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

    /* ------------------- EXECUTE SELECT ------------------- */
    json QueryRunnerSQ3::select(QueryPlannerContextSelect&& data) {
        std::stringstream sql;

        auto expanded = expandObjectProperties(data);

        addSelectClause(sql, data.select_value);
        addFromClause(sql, data);
        addWhereClause(sql, data);
        addGroupByClause(sql, data.groupBy_value);
        addOrderByClause(sql, data.sortBy_value);
        if (data.pagination_value)
            addPaginationClause(sql, data.pagination_value.value());

        // execute it
        auto reader = this->connection->executeReader(sql.str(), {});
        std::shared_ptr<IDBRowReader> row;
        while (reader->readRow(row)) {
            NLDB_TRACE(":) \n");
        }

        // read output and build object
        return {};
    }
}  // namespace nldb