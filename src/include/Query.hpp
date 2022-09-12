#pragma once
#include <tuple>

#include "Collection.hpp"
#include "PropertyRep.hpp"
#include "dbwrapper/IDB.hpp"

class Query {
   public:
    Query(const Collection& cl);

   public:
    // lets suppose that all are PropertyRep
    template <class... PropertyRep>
    auto select(const PropertyRep&... props) {}

   private:
    std::string query;
    Collection cl;
};

class CollectionQueryFactory {
   public:
    CollectionQueryFactory(const Collection& cl);

   public:
    template <class... F>
    auto properties(const F&... props) {
        auto representations =
            std::make_tuple(Query(cl), PropertyRep(props)...);

        return representations;
    }

    void insert();

   private:
    Collection cl;
};

class QueryFactory {
   public:
    static CollectionQueryFactory create(IDB& ctx, const std::string& docName);
};
