#include "nldb/DAL/Cache/CachedRepositoryPropertyCache.hpp"
#include "nldb/LOG/log.hpp"
#include "nldb/Property/Property.hpp"

namespace nldb {
    CachedRepositoryProperty::CachedRepositoryProperty(
        IDB* connection, std::unique_ptr<IRepositoryProperty> pRepo)
        : conn(connection), repo(std::move(pRepo)), cache(300, 200) {}

    snowflake CachedRepositoryProperty::add(const std::string& name) {
        return repo->add(name);
    }

    snowflake CachedRepositoryProperty::add(const std::string& name,
                                            snowflake collectionID,
                                            PropertyType type) {
        snowflake id = repo->add(name, collectionID, type);

        cache.insert({collectionID, name},
                     Property(id, name, type, collectionID));

        return id;
    }

    std::optional<Property> CachedRepositoryProperty::find(snowflake propID) {
        auto found = cache.findCopy(
            [&propID](auto& k, Property& v) { return v.getId() == propID; });

        if (found) {
            NLDB_PERF_SUCCESS("-- PROP -- cache HIT");
            return found.value();
        } else {
            auto prop = repo->find(propID);

            if (prop) {
                cache.insert({prop->getCollectionId(), prop->getName()},
                             prop.value());
            }

            return prop;
        }
    }

    std::optional<Property> CachedRepositoryProperty::find(
        snowflake collectionID, const std::string& propName) {
        if (propName == common::internal_id_string)
            return Property(-1, "_id", PropertyType::ID, collectionID);

        if (cache.contains({collectionID, propName})) {
            NLDB_PERF_SUCCESS("-- PROP -- cache HIT");
            return cache.get({collectionID, propName});
        } else {
            auto prop = this->repo->find(collectionID, propName);

            // do not cache missing props because they can be inserted later,
            // else we would need to invalidate the whole cache
            if (prop) {
                NLDB_PERF_FAIL("-- PROP -- cache MISS AND FOUND");

                // we missed a prop and probably will miss more so just cache
                // all its props.

                auto props = this->findAll(collectionID);

                for (auto& pFound : props) {
                    cache.insert({collectionID, pFound.getName()}, pFound);
                }

            } else {
                NLDB_PERF_SUCCESS("-- PROP -- cache MISS [NOT FOUND]");
            }

            return prop;
        }
    }

    bool CachedRepositoryProperty::exists(snowflake collectionID,
                                          const std::string& propName) {
        return this->find(collectionID, propName).has_value();
    }

    std::vector<Property> CachedRepositoryProperty::findAll(
        snowflake collectionId) {
        return repo->findAll(collectionId);
    }
}  // namespace nldb