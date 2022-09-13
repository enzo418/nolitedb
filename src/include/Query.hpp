#pragma once
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <type_traits>

#include "Collection.hpp"
#include "Concepts.hpp"
#include "MoveOnlyFunction.h"
#include "PropertyRep.hpp"
#include "dbwrapper/IDB.hpp"
#include "dbwrapper/ParamsBind.hpp"
#include "logger/Logger.h"
#include "nlohmann/json.hpp"

using namespace nlohmann;

template <typename Q>
concept IsPropertyRep = std::is_same<Q, PropertyRep>::value;

struct QueryCtx {
    QueryCtx(IDB* db, const Collection& cl) : db(db), cl(cl) {}

    std::stringstream sql;
    Paramsbind bind;
    Collection cl;
    IDB* db;

    void resetQuery() {
        sql.str("");
        bind = {};
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
                std::vector<PropertyRep>&& properties);

   public:
    SelectQuery& where(const SqlStatement<std::string>&);

   protected:
    void joinValues();

   private:
    std::vector<PropertyRep> properties;
    const char* documentTableAlias = "__doc";
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

    // lets suppose that all are PropertyRep for now but we could have
    // sum(prop), ...
    template <IsPropertyRep... Q>
    SelectQuery select(const Q&... props) {
        qctx->resetQuery();

        qctx->sql << "select ";

        std::vector<PropertyRep> unpackedProps = {props...};

        if (sizeof...(props) > 0) {
            for (const PropertyRep& prop : unpackedProps) {
                qctx->sql << utils::paramsbind::parseSQL(
                                 "@table.value as @prop_name",
                                 {{"@table", prop.getStatement()},
                                  {"@prop_name", std::string(prop.getName())}})
                          << ",";
            }

            // set cursor at , so we overwrite it later
            qctx->sql.seekp(-1, std::ios_base::end);
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
