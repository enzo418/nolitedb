#pragma once
#include <tuple>

#include "PropertyRep.hpp"

enum PropertyType { NUMBER = 1, STRING };

class Query {
   public:
    Query() {}

   public:
    // lets suppose that all are PropertyRep
    template <class... PropertyRep>
    auto select(const PropertyRep&... props) {}

   private:
    std::string query;
};

class QueryFactory {
   public:
    template <class... F>
    auto properties(const F&... props) {
        auto representations = std::make_tuple(Query(), PropertyRep(props)...);

        return representations;
    }
};
