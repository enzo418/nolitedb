#pragma once

#include <string>

#include "Enums.hpp"
#include "dbwrapper/IDB.hpp"
#include "lrucache11/LRUCache11.hpp"

class Collection {
   public:
    Collection(IDB* ctx, int id, const std::string& name);

   public:
    int getID();

    bool hasProperty(const std::string& key);
    bool addProperty(const std::string& key, PropertyType type);
    std::optional<int> getPropertyID(const std::string& key);

   public:
    static Collection find(IDB* ctx, const std::string& name);
    static int create(IDB& ctx, const std::string& name);

   private:
    int id;
    std::string name;
    IDB* ctx;

   private:
    void updatePropCache(const std::string& key, int propId);

    static lru11::Cache<int, std::unordered_map<std::string, int>>
        propertyCache;
};