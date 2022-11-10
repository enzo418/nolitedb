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
        QueryPlannerSelect& groupBy(const PR&... cols) {
            this->context.groupBy_value = {cols...};

            return *this;
        }

        template <IsSortedProperty... PR>
        QueryPlannerSelect& sortBy(const PR&... cols) {
            this->context.sortBy_value = {cols...};

            return *this;
        }

        /**
         * @brief Suppress properties of the resulting query, that is, it is
         * possible to use them in a where/group/sort but they will not appear
         * in the resulting json object.
         *
         * @tparam PR
         * @param props properties to suppress
         * @return QueryPlannerSelect&
         */
        template <IsProperty... PR>
        QueryPlannerSelect& suppress(const PR&... props) {
            this->context.suppress_value = {props...};

            return *this;
        }

        json execute();

       protected:
        QueryPlannerContextSelect context;
    };
};  // namespace nldb