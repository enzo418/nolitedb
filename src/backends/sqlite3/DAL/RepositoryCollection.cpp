#include "RepositoryCollection.hpp"

#include <optional>

#include "nldb/Collection.hpp"
#include "nldb/Property/Property.hpp"

namespace nldb {
    RepositoryCollection::RepositoryCollection(IDB* connection)
        : conn(connection) {}

    snowflake RepositoryCollection::add(const std::string& name,
                                        snowflake ownerID) {
        const std::string sql =
            "insert into collection (name, owner_id) values (@name, @ow_id);";
        conn->execute(sql, {{"@name", name}, {"@ow_id", ownerID}});
        return conn->getLastInsertedRowId();
    }

    std::optional<Collection> RepositoryCollection::find(
        const std::string& name) {
        const std::string sql = "select id from collection where name = @name;";

        auto id = conn->executeAndGetFirstInt(sql, {{"@name", name}});

        if (id.has_value()) {
            return Collection(id.value(), name);
        } else {
            return std::nullopt;
        }
    }

    bool RepositoryCollection::exists(const std::string& name) {
        return find(name).has_value();
    }

    std::optional<Collection> RepositoryCollection::find(snowflake id) {
        const std::string sql = "select name from collection where id = @id;";

        auto reader = conn->executeReader(sql, {{"@id", id}});

        std::shared_ptr<IDBRowReader> row;
        if (reader->readRow(row)) {
            return Collection(id, row->readString(0));
        } else {
            return std::nullopt;
        }
    }

    bool RepositoryCollection::exists(snowflake id) {
        return this->find(id).has_value();
    }

    std::optional<Collection> RepositoryCollection::findByOwner(
        snowflake ownerID) {
        const std::string sql =
            "select id, name from collection where owner_id = @owner_id;";

        auto reader = conn->executeReader(sql, {{"@owner_id", ownerID}});

        std::shared_ptr<IDBRowReader> row;
        if (reader->readRow(row)) {
            return Collection(row->readInt64(0), row->readString(1));
        } else {
            return std::nullopt;
        }
    }

    std::optional<snowflake> RepositoryCollection::getOwnerId(
        snowflake collID) {
        const std::string sql =
            "select owner_id from collection where id = @coll_id;";

        return conn->executeAndGetFirstInt(sql, {{"@coll_id", collID}});
    }
}  // namespace nldb
