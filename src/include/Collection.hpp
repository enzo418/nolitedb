#pragma once

#include <string>

#include "dbwrapper/IDB.hpp"

class Collection {
   public:
    Collection(int id, const std::string& name);

   public:
    static Collection find(IDB& ctx, const std::string& name);
    static int create(IDB& ctx, const std::string& name);
};