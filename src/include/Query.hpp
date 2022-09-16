#pragma once
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <variant>

#include "Collection.hpp"
#include "Concepts.hpp"
#include "Enums.hpp"
#include "MoveOnlyFunction.h"
#include "PropertyRep.hpp"
#include "SqlExpression.hpp"
#include "dbwrapper/IDB.hpp"
#include "dbwrapper/ParamsBind.hpp"
#include "logger/Logger.h"
#include "nlohmann/json.hpp"

using namespace nlohmann;

template <typename Q>
concept IsPropertyRep = std::is_same<Q, PropertyRep>::value;

template <typename Q>
concept IsSelectProperty =
    IsPropertyRep<Q> || std::is_same<Q, AggregateFunction>::value;

typedef std::variant<PropertyRep, AggregateFunction> SelectProperty;

/**
 * @brief Holds data to build a SQL Select query with all the possible clauses
 * that you can have.
 *
 */
struct SelectQueryData {
    const char* documentTableAlias = "__doc";

    std::stringstream select;
    std::stringstream from_join;
    std::stringstream where;
    std::stringstream groupBy;
    std::stringstream having;
    std::stringstream limit_offset;

    // properties to which the union of their tables has already been added to
    // the from clause
    std::vector<PropertyRep> propsWithJoin;

   private:
    friend class SelectQuery;
    friend class QueryCtx;

    void addFromClause();
    // adds a join clausure for a property if it wasn't added.
    void addJoinClauseIfNotExists(PropertyRep&);
    void addJoinClauses(std::vector<SelectProperty>&);
    void addJoinClauses(std::vector<PropertyRep>&);
    void addJoinClauses(const std::set<PropertyRep*>&);

    bool propAlreadyHasJoin(PropertyRep&);

    void resetQuery() {
        select.str("");
        from_join.str("");
        where.str("");
        groupBy.str("");
        having.str("");
        limit_offset.str("");

        propsWithJoin = {};
    }
};

/**
 * @brief Hold enough data to make a SQL query to a database.
 * An instance of this class gets passed between classes like in a state
 * pattern.
 */
struct QueryCtx {
    QueryCtx(IDB* db, const Collection& cl) : db(db), cl(cl) {}

    std::unique_ptr<SelectQueryData> selectCtx;
    std::stringstream sql;
    Paramsbind bind;
    Collection cl;
    IDB* db;

    void resetQuery() {
        sql.str("");
        bind = {};

        if (selectCtx) {
            selectCtx->resetQuery();
        }
    }

    void buildSelectQuery() {
        if (!selectCtx) {
            throw std::runtime_error(
                "Invalid query context use, 'select ctx' was not initilized");
        }

        sql.str("");  // just making sure

        sql << selectCtx->select.str() << selectCtx->from_join.str()
            << selectCtx->where.str() << selectCtx->groupBy.str()
            << selectCtx->having.str() << selectCtx->limit_offset.str();
    }
};

class BaseQuery {
   public:
    BaseQuery(const std::shared_ptr<QueryCtx>& qctx);

   protected:
    std::shared_ptr<QueryCtx> qctx;
};

template <typename R>
class ExecutableQuery : public BaseQuery {
   public:
    typedef std::function<R(QueryCtx&)> ExecFunc;

   public:
    ExecutableQuery(const std::shared_ptr<QueryCtx>& ctx, ExecFunc&& func)
        : BaseQuery(ctx) {
        executable = std::move(func);
    }

   public:
    R execute() { return executable(*qctx.get()); }

   protected:
    void setExecutableFunction(ExecFunc&& func) {
        this->executable = std::move(func);
    }

   private:
    MoveOnlyFunction<R(QueryCtx&)> executable = nullptr;
};

class SelectQuery : public ExecutableQuery<json> {
   public:
    SelectQuery(const std::shared_ptr<QueryCtx>& qctx,
                std::vector<SelectProperty>&& properties);

   public:
    SelectQuery& where(const SqlLogicExpression&);
    SelectQuery& page(int pageNumber, int elementsPerPage);

    template <IsPropertyRep... PR>
    SelectQuery& groupBy(PR&... cols) {
        this->qctx->selectCtx->groupBy << " GROUP BY ";

        std::vector<PropertyRep> props = {cols...};

        for (int i = 0; i < props.size(); i++) {
            this->qctx->selectCtx->groupBy << props[i].getStatement()
                                           << ".value";

            if (i != props.size() - 1) {
                this->qctx->selectCtx->groupBy << ", ";
            }
        }

        return *this;
    }

   private:
    std::vector<SelectProperty> properties;
};

class Query : public BaseQuery {
   public:
    Query(const std::shared_ptr<QueryCtx>& ctx);

   public:
    /**
     * @brief Get the properties needed to do a select query with fields.
     *
     * @tparam F string like
     * @param props properties to prepare
     * @return auto properties
     */
    template <StringLike... F>
    auto prepareProperties(const F&... props) {
        // use array so we can iterate them but tuple also works fine
        std::array<PropertyRep, sizeof...(props)> representations = {
            qctx->cl.getProperty(props)...};

        return representations;
    }

    // TODO: lets suppose that all are PropertyRep for now but we could have
    // sum(prop), ...
    template <IsSelectProperty... Q>
    SelectQuery select(const Q&... props) {
        if (!this->qctx->selectCtx) {
            this->qctx->selectCtx = std::make_unique<SelectQueryData>();
        }

        qctx->resetQuery();

        this->qctx->selectCtx->select << "select ";

        // to variant
        std::vector<SelectProperty> unpackedProps = {props...};

        if (sizeof...(props) > 0) {
            for (int i = 0; i < unpackedProps.size(); i++) {
                const auto& v_prop = unpackedProps[i];

                if (std::holds_alternative<PropertyRep>(v_prop)) {
                    const auto& prop = std::get<PropertyRep>(v_prop);

                    this->qctx->selectCtx->select
                        << utils::paramsbind::parseSQL(
                               "@table.value as @prop_name",
                               {{"@table", prop.getStatement()},
                                {"@prop_name", std::string(prop.getName())}});
                } else {
                    const auto& prop = std::get<AggregateFunction>(v_prop);

                    this->qctx->selectCtx->select
                        << AggregatefunctionTypeToString(prop.type) << "("
                        << utils::paramsbind::parseSQL(
                               "@table.value) as @agg_alias",
                               {{"@table", prop.prop->getStatement()},
                                {"@agg_alias", prop.alias}});
                }

                if (i != unpackedProps.size() - 1) {
                    this->qctx->selectCtx->select << ",";
                }
            }
        } else {
            // get all properties from the collection and add it to the select
            // and the unpacked props vector
            std::runtime_error("Not implemented");
        }

        return SelectQuery(this->qctx, std::move(unpackedProps));
    }

    /**
     * @brief insert documents into the collection.
     *
     * @param obj Array of json objects or just a json object
     * @return BaseQuery
     */
    ExecutableQuery<int> insert(const json& obj);

   protected:
    void buildPropertyInsert(
        std::stringstream& sql, Paramsbind& bind, json element,
        std::map<PropertyType, std::vector<std::string>>& insertMap);
};

class QueryFactory {
   public:
    static Query create(IDB* ctx, const std::string& collName);
};
