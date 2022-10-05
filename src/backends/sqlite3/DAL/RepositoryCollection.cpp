#include "RepositoryCollection.hpp"

#include <optional>

#include "nldb/Collection.hpp"
#include "nldb/Property/Property.hpp"

namespace nldb {
    RepositoryCollection::RepositoryCollection(IDB* connection)
        : conn(connection) {}

    int RepositoryCollection::add(const std::string& name) {
        const std::string sql = "insert into collection (name) values (@name);";
        conn->execute(sql, {{"@name", name}});
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

    std::optional<Collection> RepositoryCollection::find(int id) {
        const std::string sql = "select name from collection where id = @id;";

        auto reader = conn->executeReader(sql, {{"@id", id}});

        std::shared_ptr<IDBRowReader> row;
        if (reader->readRow(row)) {
            return Collection(id, row->readString(0));
        } else {
            return std::nullopt;
        }
    }

    bool RepositoryCollection::exists(int id) {
        return this->find(id).has_value();
    }
}  // namespace nldb
