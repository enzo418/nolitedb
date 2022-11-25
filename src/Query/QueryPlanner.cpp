#include "nldb/Query/QueryPlanner.hpp"

#include "nldb/typedef.hpp"

namespace nldb {
    QueryPlanner::QueryPlanner(QueryPlannerContext&& ctx)
        : context(std::move(ctx)) {}

    std::vector<std::string> QueryPlanner::insert(const json& object) {
        QueryPlannerContextInsert ctx(std::move(this->context));
        ctx.documents = std::move(object);

        return ctx.queryRunner->insert(std::move(ctx));
    }

    void QueryPlanner::update(snowflake docId, const json& newValue) {
        QueryPlannerContextUpdate ctx(std::move(this->context));
        ctx.documentID = docId;
        ctx.object = std::move(newValue);

        ctx.queryRunner->update(std::move(ctx));
    }

    void QueryPlanner::update(const std::string& docId, const json& newValue) {
        snowflake id;

        try {
            id = std::stoll(docId);
        } catch (std::logic_error& e) {
            throw std::runtime_error("invalid id");
        }

        this->update(id, newValue);
    }

    void QueryPlanner::remove(snowflake docId) {
        QueryPlannerContextRemove ctx(std::move(this->context));
        ctx.documentID = docId;

        ctx.queryRunner->remove(std::move(ctx));
    }

    void QueryPlanner::remove(const std::string& docId) {
        snowflake id;

        try {
            id = std::stoll(docId);
        } catch (std::logic_error& e) {
            throw std::runtime_error("invalid id");
        }

        this->remove(id);
    }
}  // namespace nldb