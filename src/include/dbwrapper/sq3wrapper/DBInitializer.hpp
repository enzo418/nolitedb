#pragma once
#include "dbwrapper/IDB.hpp"

class DBInitializer {
   public:
    static void createTablesAndFKeys(IDB* db);
};