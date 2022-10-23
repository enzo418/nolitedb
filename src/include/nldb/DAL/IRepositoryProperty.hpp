#pragma once

#include <optional>
#include <string>
#include <vector>

#include "nldb/Property/Property.hpp"
#include "nldb/typedef.hpp"

namespace nldb {
    class IRepositoryProperty {
       public:
        virtual snowflake add(const std::string& name) = 0;
        virtual snowflake add(const std::string& name, snowflake collectionID,
                              PropertyType type) = 0;

        virtual std::optional<Property> find(snowflake propID) = 0;
        virtual std::optional<Property> find(snowflake collectionID,
                                             const std::string& propName) = 0;
        virtual bool exists(snowflake collectionID,
                            const std::string& propName) = 0;
        virtual std::vector<Property> findAll(snowflake collectionId) = 0;
    };
}  // namespace nldb