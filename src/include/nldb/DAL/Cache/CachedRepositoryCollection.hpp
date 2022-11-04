#include "lrucache11/LRUCache11.hpp"
#include "nldb/Collection.hpp"
#include "nldb/DAL/IRepositoryCollection.hpp"
#include "nldb/DB/IDB.hpp"
#include "nldb/Utils/Hash.hpp"

namespace nldb {
    class CachedRepositoryCollection : public IRepositoryCollection {
       public:
        CachedRepositoryCollection(std::unique_ptr<IRepositoryCollection> repo);

       public:
        snowflake add(const std::string& name, snowflake ownerID) override;
        std::optional<Collection> find(const std::string& name) override;
        bool exists(const std::string& name) override;
        std::optional<Collection> find(snowflake id) override;
        bool exists(snowflake id) override;
        std::optional<Collection> findByOwner(snowflake ownerID) override;
        std::optional<snowflake> getOwnerId(snowflake collID) override;

       private:
        std::unique_ptr<IRepositoryCollection> repo;
        lru11::Cache<std::pair<snowflake, std::string>, Collection, pairhash>
            cache_find;
        lru11::Cache<snowflake, Collection> cache_by_owner;
        lru11::Cache<snowflake, snowflake> cache_owner_id;
    };
}  // namespace nldb