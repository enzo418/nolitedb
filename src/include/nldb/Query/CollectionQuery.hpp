#pragma once

#include <array>
#include <concepts>
#include <optional>
#include <string>

#include "nldb/Collection.hpp"
#include "nldb/DAL/Repositories.hpp"
#include "nldb/Exceptions.hpp"
#include "nldb/Property/Property.hpp"

namespace nldb {
    template <class T>
    concept StringLike = std::is_convertible_v<T, std::string_view>;

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

       public:
        template <StringLike... F>
        auto get(const F&... names) {
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

            try {
                std::array<Property, sizeof...(names)> properties = {
                    repositories->repositoryProperty->find(this->id, names)
                        .value()...};

                return properties;
            } catch (const std::bad_optional_access& e) {
                throw PropertyNotFound();
            }
        }

       private:
        std::optional<std::string> alias;
        Repositories* repositories;
    };
}  // namespace nldb