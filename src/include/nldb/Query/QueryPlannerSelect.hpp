#pragma once

#include <concepts>
#include <type_traits>

#include "nldb/Property/Property.hpp"
#include "nldb/Property/SortedProperty.hpp"
#include "nldb/Query/QueryContext.hpp"

namespace nldb {

    template <typename T>
    concept IsProperty = std::is_same<T, Property>::value;

    template <typename T>
    concept IsSortedProperty = std::is_same<T, SortedProperty>::value;

    class QueryPlannerSelect {
       public:
        QueryPlannerSelect(QueryPlannerContextSelect&& context);

       public:
        /**
         * @brief filters elements by some condition.
         * Multiple calls to this functions results in an "AND" between the
         * conditions.
         *
         * @return QueryPlannerSelect&
         */
        QueryPlannerSelect& where(const PropertyExpression&);

        QueryPlannerSelect& page(int pageNumber, int elementsPerPage);

        template <IsProperty... PR>
        QueryPlannerSelect& groupBy(PR&... cols) {
            this->context.groupBy_value = {cols...};

            return *this;
        }

        template <IsSortedProperty... PR>
        QueryPlannerSelect& sortBy(PR&... cols) {
            this->context.sortBy_value = {cols...};

            return *this;
        }

        json execute();

       protected:
        QueryPlannerContextSelect context;
    };
};  // namespace nldb