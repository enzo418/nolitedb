#include "nldb/DAL/IRepositoryCollection.hpp"
#include "nldb/DB/IDB.hpp"

namespace nldb {
    class RepositoryCollection : public IRepositoryCollection {
       public:
        RepositoryCollection(IDB* connection);

       public:
        int add(const std::string& name) override;
        std::optional<Collection> find(const std::string& name) override;
        bool exists(const std::string& name) override;
        std::optional<Collection> find(int id) override;
        bool exists(int id) override;

       private:
        IDB* conn;
    };
}  // namespace nldb