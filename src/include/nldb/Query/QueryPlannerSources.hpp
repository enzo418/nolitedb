#pragma once

#include "nldb/Query/CollectionQuery.hpp"
#include "nldb/Query/QueryContext.hpp"
#include "nldb/Query/QueryPlanner.hpp"

namespace nldb {
    class QueryPlannerSources : public QueryPlanner {
       public:
        QueryPlannerSources(QueryPlannerContext&& ctx);

       public:
        QueryPlannerSources& with(const char* collection);
        QueryPlannerSources& with(CollectionQuery collection);
    };
}  // namespace nldb