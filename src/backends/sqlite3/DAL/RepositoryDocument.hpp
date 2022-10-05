#pragma once

#include "nldb/Collection.hpp"
#include "nldb/DAL/IRepositoryDocument.hpp"
#include "nldb/DAL/Repositories.hpp"
#include "nldb/DB/IDB.hpp"

namespace nldb {
    class RepositoryDocument : public IRepositoryDocument {
       public:
        RepositoryDocument(IDB* connection);

       public:
        int add(int collid) override;

        void remove(int id) override;

        bool exists(int id) override;

       private:
        IDB* conn;
    };
}  // namespace nldb