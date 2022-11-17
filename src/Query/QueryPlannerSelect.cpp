#include "nldb/Query/QueryPlannerSelect.hpp"

#include "nldb/Property/Property.hpp"

namespace nldb {
    QueryPlannerSelect::QueryPlannerSelect(QueryPlannerContextSelect&& pContext)
        : context(std::move(pContext)) {}

    QueryPlannerSelect& QueryPlannerSelect::where(
        const PropertyExpression& val) {
        if (this->context.where_value.has_value()) {
            this->context.where_value =
                PropertyExpression(PropertyExpressionOperator::AND, val,
                                   this->context.where_value.value());
        } else {
            this->context.where_value = val;
        }

        return *this;
    }

    QueryPlannerSelect& QueryPlannerSelect::page(int pageNumber) {
        if (this->context.pagination_value.has_value()) {
            this->context.pagination_value->pageNumber = pageNumber;
        } else {
            this->context.pagination_value = {.pageNumber = pageNumber,
                                              .elementsPerPage = 10};
        }

        return *this;
    }

    QueryPlannerSelect& QueryPlannerSelect::limit(int elementsPerPage) {
        this->context.pagination_value = {
            .pageNumber = this->context.pagination_value.has_value()
                              ? this->context.pagination_value->pageNumber
                              : 1,
            .elementsPerPage = elementsPerPage};

        return *this;
    }

    json QueryPlannerSelect::execute() {
        return context.queryRunner->select(std::move(context));
    }
}  // namespace nldb