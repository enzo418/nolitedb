#pragma once

#include <optional>
#include <type_traits>

#include "QueryPlannerSelect.hpp"
#include "nldb/Object.hpp"
#include "nldb/Property/Property.hpp"
#include "nldb/nldb_json.hpp"

namespace nldb {
    template <typename T>
    concept IsSelectProperty =
        IsProperty<T> || std::is_same<T, AggregatedProperty>::value ||
        std::is_same<T, Object>::value || std::is_same<T, Collection>::value;

    template <typename T>
    inline auto toSelectable(const T& v) {
        return v;
    }

    template <>
    inline auto toSelectable<Collection>(const Collection& v) {
        return Property(v.getName(), std::nullopt);
    }

    class QueryPlanner {
       public:
        QueryPlanner(QueryPlannerContext&& ctx);

       public:
        template <IsSelectProperty... PropOrAggregatedProp>
        QueryPlannerSelect select(const PropOrAggregatedProp&... props) {
            QueryPlannerContextSelect ctx(std::move(context));
            ctx.select_value = {toSelectable(props)...};

            return QueryPlannerSelect(std::move(ctx));
        }

        void insert(json object);

        /**
         * @brief Update a document.
         * You can update every property and sub-property of the document.
         *
         * Updating sub-properties
         * -----------------------
         *  make sure to indicate the parent property that if it's a
         *  sub-property, e.g. to update the address from the collection user
         * {name, contact: {address}}you must pass
         *  {{"contact", {{"address", "new_value"}}}}.
         *
         * If a property doesn't exists
         * ----------------------------
         *  In this case a new property (and all its sub-properties if it's an
         *  object) for the document collection is added
         *
         * @param docId
         * @param newValue
         */
        void update(snowflake docId, json newValue);
        void remove(snowflake docId);

       protected:
        QueryPlannerContext context;
    };
}  // namespace nldb