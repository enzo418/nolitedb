#pragma once

#include <optional>
#include <string>
#include <tuple>

#include "nldb/Common.hpp"
#include "nldb/Object.hpp"
#include "nldb/typedef.hpp"

namespace nldb {

    template <class T>
    concept StringLike = std::is_convertible_v<T, std::string_view>;

    template <class T>
    concept StringLikeOrObjectPlaceholder =
        StringLike<T> || std::is_same<T, ObjectExpression>::value;

    constexpr ObjectExpression operator"" _obj(const char* s,
                                               std::size_t size) {
        // a new class is needed because ComposedProperty can't have a constexpr
        // ctor.
        return ObjectExpression {s, size};
    }

    /**
     * @brief This class represents the so-called root collections.
     * Giving them functionality to query inner objects and properties.
     */
    class Collection {
       public:
        Collection(const std::string& name);
        Collection(snowflake id, const std::string& name);

       public:
        snowflake getId() const;
        std::string getName() const;

       public:
        /**
         * @brief Prepares properties to be used in queries.
         *
         * You can filter fields of a subcollection using "<coll>{A, B{t}}"_obj.
         * In this case, in a select you would retrieve the properties A and B
         * of the 'coll' collection, being B composed only by the field t and A
         * with all its fields if it is an object or with its value otherwise.
         *
         * @tparam F
         * @param names can be a string, indicating the property name from the
         * collection, or an ComposedPropertyPlaceholder, which is a placeholder
         * for an object (property with more properties) that later is
         * evaluated.
         * @return auto
         */
        template <StringLikeOrObjectPlaceholder... F>
        auto get(const F&... names) {
            return std::make_tuple(toPropertyOrComposed(names)...);
        }

        template <StringLikeOrObjectPlaceholder... F>
        Object group(const F&... names) {
            return Object(Property(this->name, std::nullopt),
                          {toPropertyOrComposed(names)...});
        }

       public:
        Collection& as(std::string alias);

        std::optional<std::string> getAlias() const;

       protected:
        template <StringLike F>
        Property toPropertyOrComposed(F str) {
            return Property(str, this->name);
        }

        Object toPropertyOrComposed(const ObjectExpression& expr) {
            return Object::evaluateExpresion(expr, this->name);
        }

       protected:
        snowflake id;
        std::string name;
        std::optional<std::string> alias;
    };
}  // namespace nldb