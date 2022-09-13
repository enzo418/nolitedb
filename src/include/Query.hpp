#pragma once
#include <tuple>

#include "Collection.hpp"
#include "PropertyRep.hpp"
#include "dbwrapper/IDB.hpp"
#include "nlohmann/json.hpp"

using namespace nlohmann;

class Query {
   public:
    Query(IDB* ctx, const Collection& cl);

   public:
    // lets suppose that all are PropertyRep
    template <class... PropertyRep>
    auto select(const PropertyRep&... props) {}

   private:
    std::string query;
    Collection cl;
    IDB* ctx;
};

class CollectionQueryFactory {
   public:
    CollectionQueryFactory(IDB* ctx, const Collection& cl);

   public:
    template <class... F>
    auto properties(const F&... props) {
        auto representations =
            std::make_tuple(Query(ctx, cl), PropertyRep(props)...);

        return representations;
    }

    /**
     * @brief insert documents into the collection.
     *
     * @param obj Array of json objects or just a json object
     */
    void insert(const json& obj);

   protected:
    void buildPropertyInsert(
        std::stringstream& sql, Paramsbind& bind, json element,
        std::map<PropertyType, std::vector<std::string>>& insertMap);

   private:
    Collection cl;
    IDB* ctx;
};

class QueryFactory {
   public:
    static CollectionQueryFactory create(IDB* ctx, const std::string& collName);
};
