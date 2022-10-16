#include "lrucache11/LRUCache11.hpp"
#include "nldb/Collection.hpp"
#include "nldb/DAL/IRepositoryCollection.hpp"
#include "nldb/DB/IDB.hpp"
#include "nldb/Utils/Hash.hpp"

namespace nldb {
    class CachedRepositoryCollection : public IRepositoryCollection {
       public:
        CachedRepositoryCollection(IDB* connection,
                                   std::unique_ptr<IRepositoryCollection> repo);

       public:
        int add(const std::string& name, int ownerID) override;
        std::optional<Collection> find(const std::string& name) override;
        bool exists(const std::string& name) override;
        std::optional<Collection> find(int id) override;
        bool exists(int id) override;
        std::optional<Collection> findByOwner(int ownerID) override;
        std::optional<int> getOwnerId(int collID) override;

       private:
        IDB* conn;
        std::unique_ptr<IRepositoryCollection> repo;
        lru11::Cache<std::pair<int, std::string>, Collection, pairhash>
            cache_find;
        lru11::Cache<int, Collection> cache_by_owner;
        lru11::Cache<int, int> cache_owner_id;
    };
}  // namespace nldb