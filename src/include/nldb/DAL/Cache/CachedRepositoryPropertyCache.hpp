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
        int add(const std::string& name) override;
        int add(const std::string& name, int collectionID,
                PropertyType type) override;
        std::optional<Property> find(int collectionID,
                                     const std::string& propName) override;
        bool exists(int collectionID, const std::string& propName) override;
        std::vector<Property> find(int collectionId) override;

       private:
        IDB* conn;
        std::unique_ptr<IRepositoryProperty> repo;
        lru11::Cache<std::pair<int, std::string>, Property, pairhash> cache;
    };
}  // namespace nldb