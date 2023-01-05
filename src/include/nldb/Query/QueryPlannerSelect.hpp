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

        /**
         * @brief Get elements starting from this page.
         * First page is 1. If you don't set `limit` it will default to 10.
         *
         * @param pageNumber
         * @return QueryPlannerSelect&
         */
        QueryPlannerSelect& page(int pageNumber);

        QueryPlannerSelect& limit(int elementsPerPage);

        /**
         * @brief This will make each document inside the collection document
         * also show its own _id.
         * Because by default we return the document with the data plus ONLY its
         * _id.
         *
         * @return QueryPlannerSelect&
         */
        QueryPlannerSelect& includeInnerIds();

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

        /**
         * @brief Rename a property, it only affects the returned object.
         * Therefore in the select, where and so on you should refer to the
         * property by its original name.
         *
         * @param prop
         * @param name
         * @return QueryPlannerSelect&
         */
        QueryPlannerSelect& rename(const Property& prop,
                                   const std::string& name) {
            this->context.renamed_value.push_back({prop, name});

            return *this;
        }

        json execute();

       protected:
        QueryPlannerContextSelect context;
    };
};  // namespace nldb