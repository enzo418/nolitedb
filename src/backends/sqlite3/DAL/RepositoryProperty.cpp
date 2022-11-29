#include "nldb/backends/sqlite3/DAL/RepositoryProperty.hpp"

#include "nldb/Property/Property.hpp"

namespace nldb {
    RepositoryProperty::RepositoryProperty(IDB* connection)
        : conn(connection) {}

    snowflake RepositoryProperty::add(const std::string& name) {
        const std::string sql =
            "insert into property (name, type) values (@name, @type);";

        conn->execute(sql, {{"@name", name}, {"@type", PropertyType::OBJECT}});

        return conn->getLastInsertedRowId();
    }

    snowflake RepositoryProperty::add(const std::string& name,
                                      snowflake collectionID,
                                      PropertyType type) {
        const std::string sql =
            "insert into property (coll_id, name, type) values (@colid, @name, "
            "@type);";

        conn->execute(
            sql,
            {{"@colid", collectionID}, {"@name", name}, {"@type", (int)type}});

        return conn->getLastInsertedRowId();
    }

    std::optional<Property> RepositoryProperty::find(snowflake propID) {
        auto reader = conn->executeReader(
            "SELECT name, coll_id, type FROM property where id = @id;",
            {{"@id", propID}});

        std::shared_ptr<IDBRowReader> row;
        if (reader->readRow(row)) {
            return Property(propID, row->readString(0),
                            (PropertyType)row->readInt32(2),
                            row->isNull(1) ? -1 : row->readInt64(1));
        } else {
            return std::nullopt;
        }
    }

    std::optional<Property> RepositoryProperty::find(
        snowflake collectionID, const std::string& propName) {
        if (propName == common::internal_id_string)
            return Property(-1, "_id", PropertyType::ID, collectionID);

        auto reader = conn->executeReader(
            "SELECT id, type FROM property where coll_id = @colid and name = "
            "@name;",
            {{"@colid", collectionID}, {"@name", propName}});

        std::shared_ptr<IDBRowReader> row;
        if (reader->readRow(row)) {
            return Property(row->readInt64(0), propName,
                            (PropertyType)row->readInt32(1), collectionID);
        } else {
            return std::nullopt;
        }
    }

    bool RepositoryProperty::exists(snowflake collectionID,
                                    const std::string& propName) {
        return this->find(collectionID, propName).has_value();
    }

    std::vector<Property> RepositoryProperty::findAll(snowflake collectionId) {
        const std::string sql =
            "select id, name, type from property where coll_id = @id;";

        auto reader = conn->executeReader(sql, {{"@id", collectionId}});

        std::vector<Property> props = {};
        std::shared_ptr<IDBRowReader> row;
        while (reader->readRow(row)) {
            props.push_back(Property(row->readInt64(0), row->readString(1),
                                     (PropertyType)row->readInt64(2),
                                     collectionId));
        }

        if (!props.empty())
            props.push_back(Property(-1, "id", PropertyType::ID, collectionId));

        return props;
    }
}  // namespace nldb