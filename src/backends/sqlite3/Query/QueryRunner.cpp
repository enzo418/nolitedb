#include "nldb/backends/sqlite3/Query/QueryRunner.hpp"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>

#include "magic_enum.hpp"
#include "nldb/Collection.hpp"
#include "nldb/Common.hpp"
#include "nldb/DAL/Repositories.hpp"
#include "nldb/Exceptions.hpp"
#include "nldb/LOG/log.hpp"
#include "nldb/Object.hpp"
#include "nldb/Profiling/Profiler.hpp"
#include "nldb/Property/AggregatedProperty.hpp"
#include "nldb/Property/Property.hpp"
#include "nldb/Property/PropertyExpression.hpp"
#include "nldb/Property/SortedProperty.hpp"
#include "nldb/Query/QueryContext.hpp"
#include "nldb/Utils/Enums.hpp"
#include "nldb/Utils/ParamsBindHelpers.hpp"
#include "nldb/Utils/Variant.hpp"
#include "nldb/backends/sqlite3/DAL/Definitions.hpp"
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

    inline bool equal(const Property& a, const Property& b) {
        // -1 = -1 !! compare by collection id
        if (b.getType() == PropertyType::ID) {
            return a.getType() == PropertyType::ID &&
                   b.getCollectionId() == a.getCollectionId();
        }

        // else compare by property id
        return a.getId() == b.getId();
    }

    /* -------------- FILTER OUT EMPTY OBJECTS -------------- */
    void filterOutEmptyObjects(std::list<SelectableProperty>& data) {
        NLDB_PROFILE_FUNCTION();

        auto isEmpty =
            overloaded {[](Object& composed) {
                            return composed.getPropertiesRef().empty();
                        },
                        [](const Property& prop) {
                            // something went wrong at the expansion,
                            // delete it
                            return prop.getType() == OBJECT;
                        },
                        [](const auto&) { return false; }};

        data.remove_if(
            [&isEmpty](auto& prop) { return std::visit(isEmpty, prop); });
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

        Object composed(prop, {}, subColl->getId());

        auto expanded = repos->repositoryProperty->findAll(subColl->getId());

        // move expanded props into the composed property
        auto& props = composed.getPropertiesRef();
        props.insert_after(props.before_begin(),
                           std::make_move_iterator(expanded.begin()),
                           std::make_move_iterator(expanded.end()));

        return composed;
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

    void expandObjectProperties(const AggregatedProperty&, auto&,
                                std::shared_ptr<Repositories> const&) {}

    void expandObjectProperties(Object& composed, auto&,
                                std::shared_ptr<Repositories> const& repos) {
        auto& props = composed.getPropertiesRef();

        if (props.empty()) {
            auto expanded =
                repos->repositoryProperty->findAll(composed.getCollId());

            props.insert_after(props.before_begin(),
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
                                std::list<SelectableProperty>& select) {
        NLDB_PROFILE_FUNCTION();
        for (auto it = select.begin(); it != select.end(); it++) {
            std::visit(
                [&it, repos](auto& prop) {
                    expandObjectProperties(prop, it, repos);
                },
                *it);
        }
    }

    void expandRootProperty(std::shared_ptr<Repositories> const&,
                            std::list<SelectableProperty>&, auto&, auto&,
                            QueryRunnerCtx&) {}

    void expandRootProperty(std::shared_ptr<Repositories> const& repos,
                            std::list<SelectableProperty>& select,
                            Property& prop, auto& it, QueryRunnerCtx& ctx) {
        // If you do select(Collection("coll_name")) then we convert it to
        // Property("coll_name"), which is a root property. If that root
        // property is the same as the "from collection" then we can't convert
        // it to an object.
        if (prop.getType() == PropertyType::OBJECT) {
            auto subColl =
                repos->repositoryCollection->findByOwner(prop.getId());

            if (!subColl) {
                throw std::runtime_error("Couldn't expand property with id " +
                                         std::to_string(prop.getId()));
            }

            if (subColl->getId() == ctx.getRootCollId()) {
                // is the root prop, we need to expand it but not as
                // an object
                auto allProps =
                    repos->repositoryProperty->findAll(ctx.getRootCollId());

                select.insert(it, std::make_move_iterator(allProps.begin()),
                              std::make_move_iterator(allProps.end()));

                // set it to an empty object so it gets removed
                // later
                *it = Object(prop, {}, -1);
            }
        }
    }

    void expandRootProperty(std::shared_ptr<Repositories> const& repos,
                            std::list<SelectableProperty>& select,
                            QueryRunnerCtx& ctx) {
        for (auto it = select.begin(); it != select.end(); it++) {
            std::visit(
                [&it, &repos, &select, &ctx](auto& p) {
                    expandRootProperty(repos, select, p, it, ctx);
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

        for (auto it = props.begin(); it != props.end(); it++) {
            // why don't we use the composed.collId?
            // because we aren't building the select for ourself, we are
            // building it for `currentSelectCollId`.
            std::visit(
                [&sql, &ctx, currentSelectCollId](auto& p) {
                    addSelectClause(sql, p, ctx, currentSelectCollId);
                },
                *it);

            if (std::next(it) != props.end()) {
                sql << ", ";
            }
        }
    }

    void addSelectClause(std::stringstream& sql,
                         std::list<SelectableProperty>& props,
                         QueryRunnerCtx& ctx) {
        NLDB_PROFILE_FUNCTION();
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
            [&sql, &ctx, &ids, &alias](const Property& prop) {
                addFromClause(sql, prop, ids, ctx, alias);
            },
            [&sql, &ctx, &ids, &alias](Object& composed2) {
                addFromClause(sql, composed2, ids, ctx, alias);
            },
            [&sql, &ctx, &ids, &alias](const AggregatedProperty& prop) {
                addFromClause(sql, prop.property, ids, ctx, alias);
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
                       std::list<SelectableProperty>& props,
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

    void addFromClause(std::stringstream& sql, QueryPlannerContextSelect& data,
                       QueryRunnerCtx& ctx) {
        NLDB_PROFILE_FUNCTION();

        // main from is document/object
        sql << " from 'object' " << doc_alias << "\n";

        // add all properties that appear in the data
        std::vector<snowflake> ids;
        addFromClause(sql, data.select_value, ids, ctx);

        // before this call, select was modified to select all the fields used
        // in where/group/sort.
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
            [&sql](const std::string& str) { sql << encloseQuotesConst(str); },
            [&sql](int val) { sql << val; }, [&sql](double val) { sql << val; },
            [&sql](const char* str) { sql << encloseQuotesConst(str); }};

        auto cbOperand = overloaded {
            [&cbConstVal](LogicConstValue const& prop) {
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
        NLDB_PROFILE_FUNCTION();
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
        NLDB_PROFILE_FUNCTION();

        if (props.empty()) return;

        sql << " GROUP BY ";

        for (size_t i = 0; i < props.size(); i++) {
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
        NLDB_PROFILE_FUNCTION();

        if (props.empty()) return;

        sql << " ORDER BY ";

        for (size_t i = 0; i < props.size(); i++) {
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

    /* ------------------- SUPPRESS FIELDS ------------------ */
    bool isSuppressed(const Property& prop, std::vector<Property>& fields) {
        // maybe just store the prop.id and then search for it instead of using
        // a lambda? No, "_id" property are used with id = -1, type = ID and the
        // only distinct value is coll_id
        return std::find_if(fields.begin(), fields.end(), [&prop](Property& a) {
                   return equal(prop, a);
               }) != fields.end();
    }

    /**
     * @tparam bool IsInner is the call coming from a composed object or the
     * root document
     * @tparam T container
     */
    template <bool IsInner = false, typename T>
    void suppressFields(T& select, std::vector<Property>& fields,
                        bool removeInnerIDs) {
        auto cb = overloaded {
            [&fields, removeInnerIDs](Object& composed) {
                // Check if there is a property of type Object == root of object
                if (isSuppressed(composed.getPropertyRef(), fields)) {
                    return true;
                }

                suppressFields<true>(composed.getPropertiesRef(), fields,
                                     removeInnerIDs);
                return false;
            },
            [&fields, removeInnerIDs](Property& prop) {
                if (prop.getType() == PropertyType::ID && removeInnerIDs) {
                    if constexpr (IsInner) {  // well... i know... shh
                        return true;
                    }
                }

                return isSuppressed(prop, fields);
            },
            [](AggregatedProperty&) { return false; }};

        select.remove_if([&cb](auto& prop) { return std::visit(cb, prop); });
    }

    /* ------------------- ADD USED FIELDS ------------------ */
    template <typename T>
    inline Object* findObjectInSelect(T& data, snowflake coll_id) {
        for (auto it = data.begin(); it != data.end(); ++it) {
            // no need to make a visitor, we are searching for an Object
            if (std::holds_alternative<Object>(*it)) {
                Object& objRef = std::get<Object>(*it);
                if (objRef.getCollId() == coll_id) {
                    return &(std::get<Object>(*it));
                } else {
                    // if the object is not what we wanted, look for it in its
                    // props.
                    auto& props = objRef.getPropertiesRef();
                    auto found_it = findObjectInSelect(props, coll_id);
                    if (found_it != nullptr) {
                        return found_it;
                    }
                }
            }
        }

        return nullptr;
    }

    Object& findOrAddObjectToSelectRecursive(
        std::list<SelectableProperty>& select_value, snowflake coll_id,
        std::shared_ptr<Repositories> const& repos, snowflake from_coll_id) {
        Object* found = findObjectInSelect(select_value, coll_id);
        if (found != nullptr) {
            // this is why we use forward_list, references are not invalidated.
            // Plus we are sure this reference will be valid because the caller
            // doesn't remove elements.
            return *found;
        }

        auto root_prop =
            repos->repositoryProperty
                ->find(repos->repositoryCollection->getOwnerId(coll_id).value())
                .value();

        Object composed(root_prop, {}, coll_id);

        if (root_prop.getCollectionId() != NullID &&
            root_prop.getCollectionId() != from_coll_id) {
            Object& parent = findOrAddObjectToSelectRecursive(
                select_value, root_prop.getCollectionId(), repos, from_coll_id);

            Object& added =
                std::get<Object>(parent.addProperty(std::move(composed)));

            return added;
        } else {
            select_value.push_back(composed);

            return std::get<Object>(select_value.back());
        }
    }

    template <typename T>
    inline bool isPropertyInList(T& select, const Property& prop) {
        auto cb = overloaded {[&prop](Property& b) { return equal(prop, b); },
                              [&prop](Object& ob) {
                                  return isPropertyInList(ob.getPropertiesRef(),
                                                          prop);
                              },
                              [](auto&) { return false; }};

        return std::find_if(select.begin(), select.end(), [&cb](auto& t) {
                   return std::visit(cb, t);
               }) != select.end();
    }

    void addToSelectIfMissingProperty(
        std::list<SelectableProperty>& select, const Property& prop,
        std::shared_ptr<Repositories> const& repos, QueryRunnerCtx& ctx,
        std::vector<Property>& suppressed) {
        if (prop.getCollectionId() == ctx.getRootCollId()) {
            if (!isPropertyInList(select, prop)) {
                select.push_front(prop);
                suppressed.push_back(prop);
            }
        } else {
            Object& object = findOrAddObjectToSelectRecursive(
                select, prop.getCollectionId(), repos, ctx.getRootCollId());

            if (!isPropertyInList(select, prop)) {
                suppressed.push_back(prop);
            }

            if (!isPropertyInList(object.getPropertiesRef(), prop)) {
                object.addProperty(prop);
            }
        }
    }

    void addUsedFields(std::list<SelectableProperty>& select,
                       const PropertyExpressionOperand& expr,
                       std::shared_ptr<Repositories> const& repos,
                       QueryRunnerCtx& ctx, std::vector<Property>& suppressed);

    void addUsedFields(std::list<SelectableProperty>& select,
                       const PropertyExpression& expr,
                       std::shared_ptr<Repositories> const& repos,
                       QueryRunnerCtx& ctx, std::vector<Property>& suppressed) {
        addUsedFields(select, expr.left, repos, ctx, suppressed);
        addUsedFields(select, expr.right, repos, ctx, suppressed);
    }

    void addUsedFields(std::list<SelectableProperty>& select,
                       const PropertyExpressionOperand& expr,
                       std::shared_ptr<Repositories> const& repos,
                       QueryRunnerCtx& ctx, std::vector<Property>& suppressed) {
        auto cbConstVal = overloaded {
            [&select, &repos, &ctx, &suppressed](const Property& prop) {
                if (!isPropertyInList(select, prop)) {
                    suppressed.push_back(prop);
                }

                addToSelectIfMissingProperty(select, prop, repos, ctx,
                                             suppressed);
            },
            [](const auto&) {}};

        auto cbOperand = overloaded {
            [&cbConstVal](LogicConstValue const& prop) {
                std::visit(cbConstVal, prop);
            },
            [&select, &repos, &ctx,
             &suppressed](box<struct PropertyExpression> const& agProp) {
                addUsedFields(select, *agProp, repos, ctx, suppressed);
            },
            [&select, &repos, &ctx,
             &suppressed](PropertyExpressionOperand const& agProp) {
                addUsedFields(select, agProp, repos, ctx, suppressed);
            }};

        std::visit(cbOperand, expr);
    }

    /**
     * @brief Makes the union of the properties of where, group, sort and select
     * into select.
     */
    void addUsedFields(QueryPlannerContextSelect& data,
                       std::shared_ptr<Repositories> const& repos,
                       QueryRunnerCtx& ctx, std::vector<Property>& suppressed) {
        // Why:
        //   Since the last design change, the from clause depends on the
        //   order in which joins are added, so we can no longer call addFrom
        //   for select, where, group and sort separately. Instead, we have to
        //   run it once with all the properties the user used in the query,
        //   which may have more than the select.

        // Context: all the properties have an id (except the ones of type ID)

        // fields added to select later will need to be ignored if they are not
        // in the original select.

        for (auto& prop : data.groupBy_value) {
            addToSelectIfMissingProperty(data.select_value, prop, repos, ctx,
                                         suppressed);
        }

        for (auto& sortedProp : data.sortBy_value) {
            addToSelectIfMissingProperty(data.select_value, sortedProp.property,
                                         repos, ctx, suppressed);
        }

        if (data.where_value) {
            addUsedFields(data.select_value, data.where_value.value(), repos,
                          ctx, suppressed);
        }
    }

    /* ------------ MOVE EMBED PROPS TO SUBOJECTS ----------- */
    // When a property from another collection is selected, e.g.
    // automaker["name"], we need to move into an object so in the resulting
    // object it looks like
    // {
    //     name: "car01",
    //     automaker: {
    //         name: "ford"
    //     },
    //     year: 2001
    // }
    // which would cause a name collision if we don't do it.
    // In addition, when only one field of an internal object is selected, we
    // have to build the whole object/collection tree in order to get that
    // field.

    void moveInnerPropsToTheirSubObjects(
        std::list<SelectableProperty>& select,
        std::shared_ptr<Repositories> const& repos, QueryRunnerCtx& ctx) {
        auto wasMoved = overloaded {
            [&select, &repos, &ctx](Property& prop) {
                if (prop.getCollectionId() != ctx.getRootCollId()) {
                    // If the parent object was not yet created then add it to
                    // the select, then move the property from select to that
                    // object
                    Object& object = findOrAddObjectToSelectRecursive(
                        select, prop.getCollectionId(), repos,
                        ctx.getRootCollId());

                    if (!isPropertyInList(object.getPropertiesRef(), prop)) {
                        object.addProperty(std::move(prop));
                    }

                    // remove property from the root select since we moved
                    // it
                    return true;
                }

                return false;
            },
            [&select, &repos, &ctx](AggregatedProperty& agProp) {
                Property& prop = agProp.property;

                if (prop.getCollectionId() != ctx.getRootCollId()) {
                    Object& object = findOrAddObjectToSelectRecursive(
                        select, prop.getCollectionId(), repos,
                        ctx.getRootCollId());

                    if (!isPropertyInList(object.getPropertiesRef(), prop)) {
                        object.addProperty(std::move(agProp));
                    }

                    return true;
                }

                return false;
            },
            [](auto&) { return false; }};

        select.remove_if(
            [&wasMoved](auto& prop) { return std::visit(wasMoved, prop); });
    }

    /* ------------------------ READ ------------------------ */
    inline void read(std::shared_ptr<IDBRowReader> row, PropertyType type,
                     json& out, const std::string& alias, int i) {
        switch (type) {
            case PropertyType::INTEGER:
                out[alias] = row->readInt64(i);
                break;
            case PropertyType::DOUBLE:
                out[alias] = row->readDouble(i);
                break;
            case PropertyType::STRING:
                out[alias] = row->readString(i);
                break;
            case PropertyType::ID:
                // javascript can't read 64 bits long integers
                out[common::internal_id_string] =
                    std::to_string(row->readInt64(i));
                break;
            case PropertyType::ARRAY:
                out[alias] = json::parse(row->readString(i));
                break;
            case PropertyType::BOOLEAN:
                out[alias] = row->readInt32(i) == 1 ? true : false;
                break;
            default:
                throw std::runtime_error(
                    "Select properties should not hold a reserved "
                    "property directly!");
                break;
        }
    }

    void read(const Property& prop, std::shared_ptr<IDBRowReader> row, int& i,
              json& out, std::vector<Property>& suppressed) {
        if (!row->isNull(i) && !isSuppressed(prop, suppressed)) {
            read(row, prop.getType(), out, prop.getName(), i);
        }

        i++;
    }

    void read(const AggregatedProperty& prop, std::shared_ptr<IDBRowReader> row,
              int& i, json& out, std::vector<Property>&) {
        read(row, prop.property.getType(), out, prop.alias, i);
        i++;
    }

    void read(Object& composed, std::shared_ptr<IDBRowReader> row, int& i,
              json& out, std::vector<Property>& suppressed) {
        const std::string& name = composed.getProperty().getName();

        json temp;

        auto& propsRef = composed.getPropertiesRef();
        for (auto& prop : propsRef) {
            std::visit([&row, &i, &temp, &suppressed](
                           auto& t) { read(t, row, i, temp, suppressed); },
                       prop);
        }

        if (!temp.is_null()) out[name] = std::move(temp);
    }

    json readQuery(std::stringstream& sql, nldb::IDB* connection,
                   QueryPlannerContextSelect& data) {
        NLDB_PROFILE_FUNCTION();
        std::unique_ptr<nldb::IDBQueryReader> reader;
        std::shared_ptr<IDBRowReader> row;

        {
            NLDB_PROFILE_SCOPE("execute reader");
            reader = connection->executeReader(sql.str(), {});
        }

        json result = json::array();
        auto begin = data.select_value.begin();
        auto end = data.select_value.end();

        while (true) {
            {
                NLDB_PROFILE_SCOPE("read row");
                if (!reader->readRow(row)) break;
            }

            NLDB_PROFILE_SCOPE("into json");

            json rowValue;
            {
                int i = 0;
                for (auto it = begin; it != end; it++) {
                    std::visit(
                        [&row, &i, &rowValue, &data](auto& val) {
                            read(val, row, i, rowValue, data.suppress_value);
                        },
                        *it);
                }
            }

            if (!rowValue.is_null()) result.push_back(std::move(rowValue));
        }

        return result;
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

    void printSelect(std::list<SelectableProperty>& list) {
        auto cb = [](auto& p) { printSelect(p, 2); };

        std::cout << "\nSelect:\n";

        for (auto& st : list) {
            std::visit(cb, st);
        }

        std::cout << "\n";
    }

    /* ------------------- EXECUTE SELECT ------------------- */
    json QueryRunnerSQ3::select(QueryPlannerContextSelect&& data) {
        NLDB_PROFILE_BEGIN_SESSION("select", "nldb-profile-select.json");

        json res;

        {
            NLDB_PROFILE_FUNCTION();

            // Check if the root collection exists
            const std::string rootCollName = data.from.begin()->getName();
            auto rootColFound = repos->repositoryCollection->find(rootCollName);
            if (!rootColFound) return res = json::array();  // empty array

            populateData<DoThrow>(data);

            selectAllOnEmpty(data, repos);

            std::stringstream sql;

            auto rootColl = repos->repositoryCollection->find(rootCollName);

            if (!rootColl) {
                return {};  // the collection doesn't even exists
            }

            QueryRunnerCtx ctx(
                rootColl->getId(),
                repos->repositoryCollection->getOwnerId(rootColl->getId())
                    .value_or(-1),
                doc_alias);

            expandRootProperty(repos, data.select_value, ctx);
            expandObjectProperties(repos, data.select_value);
            suppressFields(data.select_value, data.suppress_value,
                           data.removeInnerIDs);
            filterOutEmptyObjects(data.select_value);

            moveInnerPropsToTheirSubObjects(data.select_value, repos, ctx);
            addUsedFields(data, repos, ctx, data.suppress_value);

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
            res = readQuery(sql, connection, data);
        }

        NLDB_PROFILE_END_SESSION();
        return res;
    }
}  // namespace nldb