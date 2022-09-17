#include "Collection.hpp"

#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "Enums.hpp"
#include "PropertyRep.hpp"
#include "SqlExpression.hpp"
#include "dbwrapper/IDB.hpp"
#include "logger/Logger.h"
// max to 20 collections
lru11::Cache<int, std::unordered_map<std::string, PropertyRep>>
    Collection::propertyCache(20);

Collection::Collection(IDB* pCtx, int pId, const std::string& pName)
    : ctx(pCtx), id(pId), name(pName) {}

int Collection::getID() { return this->id; }

bool Collection::hasProperty(const std::string& key) {
    return tryGetProperty(key).has_value();
}

bool Collection::addProperty(const std::string& key, PropertyType type) {
    const std::string sql =
        "insert into property (coll_id, name, type) values (@colid, @name, "
        "@type);";

    (void)ctx->executeOneStep(
        sql, {{"@colid", this->id}, {"@name", key}, {"@type", (int)type}});

    int id = ctx->getLastInsertedRowId();

    if (id >= 0) updatePropCache(key, PropertyRep(key, id, type));

    return id >= 0;
}

std::optional<PropertyRep> Collection::tryGetProperty(const std::string& key) {
    // check cache
    if (propertyCache.contains(this->id)) {
        if (propertyCache.get(this->id).contains(key)) {
            return propertyCache.get(this->id).at(key);
        }
    }

    LogDebug("Cache miss with key '%s'", key.c_str());

    // else check db
    std::optional<int> id;
    auto property = PropertyRep::find(ctx, this->id, key);
    if (property.has_value()) {
        id = property.value().getId();
        updatePropCache(key, property.value());
    }

    return property;
}

PropertyRep Collection::getProperty(const std::string& key) {
    if (key == "id") {
        return PropertyRep("id", -1, PropertyType::ID);
    }

    auto prop = this->tryGetProperty(key);
    if (prop.has_value()) {
        return prop.value();
    } else {
        LogError("Property '%s' was requested but wasn't found.", key.c_str());

        throw std::runtime_error("Missing property");
    }
}

std::vector<PropertyRep> Collection::getAllTheProperties() {
    const std::string sql =
        "select id, name, type from property where coll_id = @id;";

    auto reader = ctx->executeReader(sql, {{"@id", this->id}});

    std::vector<PropertyRep> props = {PropertyRep("id", -1, PropertyType::ID)};
    std::shared_ptr<IDBRowReader> row;
    while (reader->readRow(row)) {
        props.push_back(PropertyRep(row->readString(1), row->readInt64(0),
                                    (PropertyType)row->readInt64(2)));
    }

    return std::move(props);
}

Collection Collection::find(IDB* ctx, const std::string& name) {
    const std::string sql = "SELECT id FROM collection where name = @name;";

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

int Collection::documentExists(int doc_id) {
    const auto sql =
        "select d.id from document as d where coll_id=@coll_id and d.id = @id;";

    auto id = ctx->executeAndGetFirstInt(
        sql, {{"@coll_id", this->id}, {"@id", doc_id}});

    return id.has_value();
}

void Collection::updatePropCache(const std::string& key, PropertyRep prop) {
    std::unordered_map<std::string, PropertyRep> map;
    propertyCache.tryGet(this->id, map);
    map.insert({key, prop});
    propertyCache.insert(this->id, map);  // updates the position
}
