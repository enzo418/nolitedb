#pragma once

#include <optional>
#include <string>
#include <vector>

#include "nldb/Common.hpp"
#include "nldb/DAL/IRepositoryProperty.hpp"
#include "nldb/DB/IDB.hpp"
#include "nldb/Property/Property.hpp"

namespace nldb {
    class RepositoryProperty : public IRepositoryProperty {
       public:
        RepositoryProperty(IDB* connection);

       public:
        snowflake add(const std::string& name) override;
        snowflake add(const std::string& name, snowflake collectionID,
                      PropertyType type) override;
        std::optional<Property> find(snowflake propID) override;
        std::optional<Property> find(snowflake collectionID,
                                     const std::string& propName) override;
        bool exists(snowflake collectionID,
                    const std::string& propName) override;
        std::vector<Property> findAll(snowflake collectionId) override;

       private:
        IDB* conn;
    };
}  // namespace nldb