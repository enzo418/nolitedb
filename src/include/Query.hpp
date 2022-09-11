#pragma once
#include <tuple>

#include "PropertyRep.hpp"

class Query {
   public:
    Query() {}
};

class QueryFactory {
   public:
    template <class... F>
    auto properties(const F&... props) {
        auto representations = std::make_tuple(Query(), PropertyRep(props)...);

        return representations;
    }
};
