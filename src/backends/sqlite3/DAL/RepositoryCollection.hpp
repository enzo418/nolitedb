#include "nldb/DAL/IRepositoryCollection.hpp"
#include "nldb/DB/IDB.hpp"

namespace nldb {
    class RepositoryCollection : public IRepositoryCollection {
       public:
        RepositoryCollection(IDB* connection);

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
    };
}  // namespace nldb