#include "nldb/DAL/Cache/CachedRepositoryCollection.hpp"

#include <optional>

#include "nldb/Collection.hpp"
#include "nldb/DAL/IRepositoryCollection.hpp"
#include "nldb/LOG/log.hpp"
#include "nldb/Property/Property.hpp"

namespace nldb {
    CachedRepositoryCollection::CachedRepositoryCollection(
        IDB* connection, std::unique_ptr<IRepositoryCollection> pRepo)
        : conn(connection), repo(std::move(pRepo)) {}

    snowflake CachedRepositoryCollection::add(const std::string& name,
                                              snowflake ownerID) {
        snowflake id = repo->add(name, ownerID);

        cache_owner_id.insert(id, ownerID);
        cache_find.insert({id, name}, Collection(id, name));

        return id;
    }

    std::optional<Collection> CachedRepositoryCollection::find(
        const std::string& name) {
        auto found = cache_find.findCopy(
            [&name](auto p) { return p.first.second == name; });

        if (found) {
            NLDB_INFO("-- COLL -- cache HIT");
            return found;
        }

        NLDB_INFO("-- COLL -- cache MISS");
        auto p = repo->find(name);
        if (p) cache_find.insert({p->getId(), name}, p.value());
        return p;
    }

    bool CachedRepositoryCollection::exists(const std::string& name) {
        return find(name).has_value();
    }

    std::optional<Collection> CachedRepositoryCollection::find(snowflake id) {
        auto found =
            cache_find.findCopy([id](auto p) { return p.first.first == id; });

        if (found) {
            NLDB_INFO("-- COLL -- cache HIT");
            return found;
        }

        NLDB_INFO("-- COLL -- cache MISS");
        auto p = repo->find(id);
        if (p) cache_find.insert({id, p->getName()}, p.value());
        return p;
    }

    bool CachedRepositoryCollection::exists(snowflake id) {
        return this->find(id).has_value();
    }

    std::optional<Collection> CachedRepositoryCollection::findByOwner(
        snowflake ownerID) {
        if (cache_by_owner.contains(ownerID)) {
            NLDB_INFO("-- BY OWNER -- cache HIT");
            return cache_by_owner.getCopy(ownerID);
        }

        NLDB_INFO("-- BY OWNER -- cache MISS");
        auto found = repo->findByOwner(ownerID);
        if (found) cache_by_owner.insert(ownerID, found.value());
        return found;
    }

    std::optional<snowflake> CachedRepositoryCollection::getOwnerId(
        snowflake collID) {
        if (cache_owner_id.contains(collID)) {
            NLDB_INFO("-- OWNER ID -- cache HIT");
            return cache_owner_id.getCopy(collID);
        }
        NLDB_INFO("-- OWNER ID -- cache MISS");

        auto found = repo->getOwnerId(collID);
        if (found) cache_owner_id.insert(collID, found.value());

        return found;
    }
}  // namespace nldb