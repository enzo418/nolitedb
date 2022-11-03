#pragma once

#include <optional>
#include <string>
#include <vector>

#include "nldb/Collection.hpp"
#include "nldb/Property/Property.hpp"
#include "nldb/typedef.hpp"

namespace nldb {
    class IRepositoryCollection {
       public:
        /**
         * @brief Adds a collection and return the id
         *
         * @param name
         * @return snowflake id
         */
        virtual snowflake add(const std::string& name, snowflake ownerID) = 0;
        virtual std::optional<Collection> find(const std::string& name) = 0;
        virtual std::optional<Collection> find(snowflake id) = 0;
        virtual bool exists(const std::string& name) = 0;
        virtual bool exists(snowflake id) = 0;
        virtual std::optional<Collection> findByOwner(snowflake ownerID) = 0;
        virtual std::optional<snowflake> getOwnerId(snowflake collID) = 0;

        virtual ~IRepositoryCollection() = default;
    };
}  // namespace nldb