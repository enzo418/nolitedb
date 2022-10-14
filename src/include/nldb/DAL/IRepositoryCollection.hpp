#pragma once

#include <optional>
#include <string>
#include <vector>

#include "nldb/Collection.hpp"
#include "nldb/Property/Property.hpp"

namespace nldb {
    class IRepositoryCollection {
       public:
        /**
         * @brief Adds a collection and return the id
         *
         * @param name
         * @return int
         */
        virtual int add(const std::string& name, int ownerID) = 0;
        virtual std::optional<Collection> find(const std::string& name) = 0;
        virtual std::optional<Collection> find(int id) = 0;
        virtual bool exists(const std::string& name) = 0;
        virtual bool exists(int id) = 0;
        virtual std::optional<Collection> findByOwner(int ownerID) = 0;
        virtual std::optional<int> getOwnerId(int collID) = 0;
    };
}  // namespace nldb