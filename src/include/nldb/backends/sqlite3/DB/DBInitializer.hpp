#pragma once
#include "nldb/DB/IDB.hpp"

namespace nldb {
    class DBInitializer {
       public:
        static void createTablesAndFKeys(IDB* db);
    };
}  // namespace nldb