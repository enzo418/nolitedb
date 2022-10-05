#include "RepositoryProperty.hpp"

#include "nldb/Property/Property.hpp"

namespace nldb {
    RepositoryProperty::RepositoryProperty(IDB* connection)
        : conn(connection) {}

    int RepositoryProperty::add(const std::string& name, int collectionID,
                                PropertyType type) {
        const std::string sql =
            "insert into property (coll_id, name, type) values (@colid, @name, "
            "@type);";

        conn->execute(
            sql,
            {{"@colid", collectionID}, {"@name", name}, {"@type", (int)type}});

        return conn->getLastInsertedRowId();
    }

    std::optional<Property> RepositoryProperty::find(
        int collectionID, const std::string& propName) {
        auto reader = conn->executeReader(
            "SELECT id, type FROM property where coll_id = @colid and name = "
            "@name",
            {{"@colid", collectionID}, {"@name", propName}});

        std::shared_ptr<IDBRowReader> row;
        if (reader->readRow(row)) {
            return Property(row->readInt32(0), propName,
                            (PropertyType)row->readInt32(1));
        } else {
            return std::nullopt;
        }
    }

    bool RepositoryProperty::exists(int collectionID,
                                    const std::string& propName) {
        return this->find(collectionID, propName).has_value();
    }

    std::vector<Property> RepositoryProperty::find(int collectionId) {
        const std::string sql =
            "select id, name, type from property where coll_id = @id;";

        auto reader = conn->executeReader(sql, {{"@id", collectionId}});

        std::vector<Property> props = {Property(-1, "id", PropertyType::ID)};
        std::shared_ptr<IDBRowReader> row;
        while (reader->readRow(row)) {
            props.push_back(Property(row->readInt64(0), row->readString(1),
                                     (PropertyType)row->readInt64(2)));
        }

        return std::move(props);
    }
}  // namespace nldb