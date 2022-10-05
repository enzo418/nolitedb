#include "nldb/Query/QueryPlannerSources.hpp"

#include "nldb/Exceptions.hpp"

namespace nldb {
    QueryPlannerSources::QueryPlannerSources(QueryPlannerContext&& ctx)
        : QueryPlanner(std::move(ctx)) {}

    QueryPlannerSources& QueryPlannerSources::with(const char* collection) {
        auto coll = context.repos.repositoryCollection->find(collection);

        if (!coll.has_value()) throw CollectionNotFound();

        this->context.from.push_back(
            CollectionQuery(&context.repos, coll.value()));

        return *this;
    }

    QueryPlannerSources& QueryPlannerSources::with(CollectionQuery collection) {
        this->context.from.push_back(collection);
        return *this;
    }
}  // namespace nldb