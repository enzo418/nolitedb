#include "nldb/Query/QueryPlanner.hpp"

namespace nldb {
    QueryPlanner::QueryPlanner(QueryPlannerContext&& ctx)
        : context(std::move(ctx)) {}

    void QueryPlanner::insert(json object) {
        QueryPlannerContextInsert ctx(std::move(this->context));
        ctx.documents = object;

        ctx.queryRunner->insert(std::move(ctx));
    }

    void QueryPlanner::update(int docId, json newValue) {
        QueryPlannerContextUpdate ctx(std::move(this->context));
        ctx.documentID = docId;
        ctx.object = newValue;

        ctx.queryRunner->update(std::move(ctx));
    }

    void QueryPlanner::remove(int docId) {
        QueryPlannerContextRemove ctx(std::move(this->context));
        ctx.documentID = docId;

        ctx.queryRunner->remove(std::move(ctx));
    }
}  // namespace nldb