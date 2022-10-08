#pragma once

#include <array>
#include <concepts>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>

#include "nldb/Collection.hpp"
#include "nldb/DAL/Repositories.hpp"
#include "nldb/Exceptions.hpp"
#include "nldb/LOG/log.hpp"
#include "nldb/Property/ComposedProperty.hpp"
#include "nldb/Property/Property.hpp"

namespace nldb {
    constexpr bool isObject(std::string_view str) { return str.find("."); }

    // constexpr ComposedProperty getP(std::string_view str) {
    //     if (isObject(str)) {
    //         return ComposedProperty(1, "str", -1, -1, {});
    //     } else {
    //         return Property(-1, "", ID);
    //     }
    // }

    template <class T>
    concept StringLike = std::is_convertible_v<T, std::string_view>;

    template <class T>
    concept StringLikeOrObjectPlaceholder =
        StringLike<T> || std::is_same<T, ComposedPropertyExpression>::value;

    constexpr ComposedPropertyExpression operator"" _obj(const char* s,
                                                         std::size_t size) {
        // a new class is needed because ComposedProperty can't have a constexpr
        // ctor.
        return ComposedPropertyExpression {s, size};
    }

    /**
     * @brief As its name indicates, it serves a placeholder to a collection
     * during the query planner stage.
     */
    class CollectionQuery : public Collection {
       public:
        CollectionQuery(Repositories* repositories, const std::string& name,
                        std::string alias = "");
        CollectionQuery(Repositories* repositories, Collection col,
                        std::string alias = "");

       public:
        CollectionQuery& as(std::string alias);

        std::optional<std::string> getAlias() const;

        /**
         * @brief Copies the collection into the internal from this class.
         */
        void setCollection(const Collection&);

        bool hasCollectionAssigned() const;

       protected:
        template <StringLike F>
        Property toPropertyOrComposed(Repositories* repositories, int id,
                                      F str) {
            return repositories->repositoryProperty->find(id, str).value();
        }

        ComposedProperty toPropertyOrComposed(Repositories* repositories,
                                              int collID,
                                              ComposedPropertyExpression expr) {
            return ComposedProperty::evaluateExpresion(collID, repositories,
                                                       expr);
        }

       public:
        /**
         * @brief Prepares properties to be used in queries.
         *
         * You can filter fields of a subcollection using "<coll>[A, B[t]]"_obj.
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
            // 1. get the collection before getting its properties
            if (!this->hasCollectionAssigned()) {
                auto coll =
                    this->repositories->repositoryCollection->find(this->name);

                if (!coll.has_value()) {
                    throw CollectionNotFound(
                        "Can't get properties from a collection that doesn't "
                        "exists");
                } else {
                    this->setCollection(coll.value());
                }
            }

            // 2. get the required properties
            try {
                return std::make_tuple((toPropertyOrComposed(
                    this->repositories, this->id, names))...);
            } catch (const std::bad_optional_access& e) {
                throw PropertyNotFound();
            }
        }

       private:
        std::optional<std::string> alias;
        Repositories* repositories;
    };
}  // namespace nldb