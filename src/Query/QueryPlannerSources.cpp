#include "nldb/Query/QueryPlannerSources.hpp"

#include "nldb/Collection.hpp"
#include "nldb/Exceptions.hpp"

namespace nldb {
    QueryPlannerSources::QueryPlannerSources(QueryPlannerContext&& ctx)
        : QueryPlanner(std::move(ctx)) {}

    QueryPlannerSources& QueryPlannerSources::with(const char* collection) {
        this->context.from.push_back(Collection(collection));

        return *this;
    }

    QueryPlannerSources& QueryPlannerSources::with(Collection collection) {
        this->context.from.push_back(collection);
        return *this;
    }
}  // namespace nldb