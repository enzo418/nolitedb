#include "nldb/DAL/IRepositoryCollection.hpp"
#include "nldb/DB/IDB.hpp"
#include "nldb/typedef.hpp"

namespace nldb {
    class RepositoryCollection : public IRepositoryCollection {
       public:
        RepositoryCollection(IDB* connection);

       public:
        snowflake add(const std::string& name, snowflake ownerID) override;
        std::optional<Collection> find(const std::string& name) override;
        bool exists(const std::string& name) override;
        std::optional<Collection> find(snowflake id) override;
        bool exists(snowflake id) override;
        std::optional<Collection> findByOwner(snowflake ownerID) override;
        std::optional<snowflake> getOwnerId(snowflake collID) override;

       private:
        IDB* conn;
    };
}  // namespace nldb