#include "Collection.hpp"

#include <stdexcept>

#include "dbwrapper/IDB.hpp"
#include "logger/Logger.h"
// max to 20 collections
lru11::Cache<int, std::unordered_map<std::string, int>>
    Collection::propertyCache(20);

Collection::Collection(IDB* pCtx, int pId, const std::string& pName)
    : ctx(pCtx), id(pId), name(pName) {}

int Collection::getID() { return this->id; }

std::optional<int> Collection::getPropertyID(const std::string& key) {
    // check cache
    if (propertyCache.contains(this->id)) {
        if (propertyCache.get(this->id).contains(key)) {
            return propertyCache.get(this->id).at(key);
        }
    }

    LogDebug("Cache miss with key %s", key.c_str());

    // else check db
    auto id = ctx->executeAndGetFirstInt(
        "SELECT id FROM property where coll_id = @colid and name = @name",
        {{"@colid", this->id}, {"@name", key}});

    if (id.has_value()) updatePropCache(key, id.value());

    return id;
}

bool Collection::hasProperty(const std::string& key) {
    return getPropertyID(key).has_value();
}

bool Collection::addProperty(const std::string& key, PropertyType type) {
    const std::string sql =
        "insert into property (coll_id, name, type) values (@colid, @name, "
        "@type)";

    (void)ctx->executeOneStep(
        sql, {{"@colid", this->id}, {"@name", key}, {"@type", (int)type}});

    int id = ctx->getLastInsertedRowId();

    if (id >= 0) updatePropCache(key, id);

    return id >= 0;
}

Collection Collection::find(IDB* ctx, const std::string& name) {
    const std::string sql = "SELECT id FROM collection where name = @name";

    auto id = ctx->executeAndGetFirstInt(sql, {{"@name", name}});

    if (id.has_value()) {
        LogTrace("Collection %s found", name.c_str());
        return Collection(ctx, id.value(), name);
    } else {
        LogTrace("Collection %s not found, creating it", name.c_str());
        return Collection(ctx, create(*ctx, name), name);
    }
}

int Collection::create(IDB& ctx, const std::string& name) {
    const std::string sql = "insert into collection (name) values (@name);";

    (void)ctx.executeOneStep(sql, {{"@name", name}});

    return ctx.getLastInsertedRowId();
}

void Collection::updatePropCache(const std::string& key, int propId) {
    std::unordered_map<std::string, int> map;
    propertyCache.tryGet(this->id, map);
    map.insert({key, propId});
    propertyCache.insert(this->id, map);  // updates the position
}