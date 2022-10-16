#include "nldb/DAL/Cache/CachedRepositoryPropertyCache.hpp"
#include "nldb/LOG/log.hpp"
#include "nldb/Property/Property.hpp"

namespace nldb {
    CachedRepositoryProperty::CachedRepositoryProperty(
        IDB* connection, std::unique_ptr<IRepositoryProperty> pRepo)
        : conn(connection), repo(std::move(pRepo)), cache(150, 10) {}

    int CachedRepositoryProperty::add(const std::string& name) {
        return repo->add(name);
    }

    int CachedRepositoryProperty::add(const std::string& name, int collectionID,
                                      PropertyType type) {
        int id = repo->add(name, collectionID, type);

        cache.insert({collectionID, name},
                     Property(id, name, type, collectionID));

        return id;
    }

    std::optional<Property> CachedRepositoryProperty::find(
        int collectionID, const std::string& propName) {
        if (propName == common::internal_id_string)
            return Property(-1, "_id", PropertyType::ID, collectionID);

        if (cache.contains({collectionID, propName})) {
            NLDB_INFO("-- PROP -- cache HIT");
            return cache.get({collectionID, propName});
        } else {
            auto prop = this->repo->find(collectionID, propName);

            // do not cache missing props because they can be inserted later,
            // else we would need to invalidate the whole cache
            if (prop) {
                NLDB_INFO("-- PROP -- cache MISS AND FOUND");

                cache.insert({collectionID, propName}, prop.value());
            } else {
                NLDB_INFO("-- PROP -- cache MISS [NOT FOUND]");
            }

            return prop;
        }
    }

    bool CachedRepositoryProperty::exists(int collectionID,
                                          const std::string& propName) {
        return this->find(collectionID, propName).has_value();
    }

    std::vector<Property> CachedRepositoryProperty::find(int collectionId) {
        return repo->find(collectionId);
    }
}  // namespace nldb