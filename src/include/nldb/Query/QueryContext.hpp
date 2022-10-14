#pragma once

#include <optional>
#include <variant>
#include <vector>

#include "nldb/DAL/Repositories.hpp"
#include "nldb/Object.hpp"
#include "nldb/Property/AggregatedProperty.hpp"
#include "nldb/Property/Property.hpp"
#include "nldb/Property/PropertyExpression.hpp"
#include "nldb/Property/SortedProperty.hpp"
#include "nldb/Query/IQueryRunner.hpp"
#include "nldb/nldb_json.hpp"

namespace nldb {
    // They are all here bc they are just DTO without any functionality.

    typedef std::variant<Property, AggregatedProperty, Object>
        SelectableProperty;

    // TODO: Rename to QueryContext
    struct QueryPlannerContext {
        std::vector<Collection> from;
        std::unique_ptr<IQueryRunner> queryRunner;
    };

    struct QueryPagination {
        int pageNumber;
        int elementsPerPage;
    };

    struct QueryPlannerContextSelect : public QueryPlannerContext {
        QueryPlannerContextSelect(QueryPlannerContext&& ctx)
            : QueryPlannerContext(std::move(ctx)) {}

        std::forward_list<SelectableProperty> select_value;
        std::optional<PropertyExpression> where_value;
        std::optional<QueryPagination> pagination_value;
        std::vector<Property> groupBy_value;
        std::vector<SortedProperty> sortBy_value;
    };

    struct QueryPlannerContextRemove : public QueryPlannerContext {
        QueryPlannerContextRemove(QueryPlannerContext&& ctx)
            : QueryPlannerContext(std::move(ctx)) {}

        int documentID;
    };

    struct QueryPlannerContextUpdate : public QueryPlannerContext {
        QueryPlannerContextUpdate(QueryPlannerContext&& ctx)
            : QueryPlannerContext(std::move(ctx)) {}

        int documentID;
        json object;
    };

    struct QueryPlannerContextInsert : public QueryPlannerContext {
        QueryPlannerContextInsert(QueryPlannerContext&& ctx)
            : QueryPlannerContext(std::move(ctx)) {}

        json documents;  // json can be an array of object
    };
}  // namespace nldb