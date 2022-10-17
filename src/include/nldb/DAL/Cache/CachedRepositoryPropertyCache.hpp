#pragma once

#include <optional>
#include <string>
#include <vector>

#include "lrucache11/LRUCache11.hpp"
#include "nldb/Common.hpp"
#include "nldb/DAL/IRepositoryProperty.hpp"
#include "nldb/DB/IDB.hpp"
#include "nldb/Property/Property.hpp"
#include "nldb/Utils/Hash.hpp"

namespace nldb {
    class CachedRepositoryProperty : public IRepositoryProperty {
       public:
        CachedRepositoryProperty(IDB* connection,
                                 std::unique_ptr<IRepositoryProperty> repo);

       public:
        snowflake add(const std::string& name) override;
        snowflake add(const std::string& name, snowflake collectionID,
                      PropertyType type) override;
        std::optional<Property> find(snowflake collectionID,
                                     const std::string& propName) override;
        bool exists(snowflake collectionID,
                    const std::string& propName) override;
        std::vector<Property> find(snowflake collectionId) override;

       private:
        IDB* conn;
        std::unique_ptr<IRepositoryProperty> repo;
        lru11::Cache<std::pair<snowflake, std::string>, Property, pairhash>
            cache;
    };
}  // namespace nldb