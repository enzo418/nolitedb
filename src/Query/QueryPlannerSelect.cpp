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
                                   this->context.where_value);
        } else {
            this->context.where_value = val;
        }

        return *this;
    }

    QueryPlannerSelect& QueryPlannerSelect::page(int pageNumber,
                                                 int elementsPerPage) {
        this->context.pagination_value = {.pageNumber = pageNumber,
                                          .elementsPerPage = elementsPerPage};

        return *this;
    }

    json QueryPlannerSelect::execute() {
        return context.queryRunner->select(std::move(context));
    }
}  // namespace nldb