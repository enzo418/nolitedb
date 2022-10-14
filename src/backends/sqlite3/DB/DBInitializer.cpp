#include "DBInitializer.hpp"

#include "nldb/LOG/log.hpp"

namespace nldb {

    void createCollectionTable(IDB* db) {
        const auto sql =
            "CREATE TABLE `collection` ("
            "`id` INTEGER PRIMARY KEY,"
            "`name` varchar(255) NOT NULL,"
            "`owner_id` INTEGER NOT NULL,"
            "FOREIGN KEY (owner_id) REFERENCES property(id)"
            ");";

        db->execute(sql, {});
    }

    void createObjectTable(IDB* db) {
        const auto sql =
            "CREATE TABLE `object` ("
            "`id` INTEGER PRIMARY KEY,"
            "`prop_id` INTEGER NOT NULL,"
            "`obj_id` INTEGER,"
            "FOREIGN KEY (prop_id) REFERENCES property(id),"
            "FOREIGN KEY (obj_id) REFERENCES object(id)"
            ");";

        db->execute(sql, {});
    }

    void createPropertyTable(IDB* db) {
        const auto sql =
            "CREATE TABLE `property` ("
            "`id` INTEGER PRIMARY KEY,"
            "`coll_id` int,"  // nullable!
            "`name` varchar(255) NOT NULL,"
            "`type` int NOT NULL,"
            "FOREIGN KEY (coll_id) REFERENCES collection(id)"
            ");";

        db->execute(sql, {});
    }

    void createValuesTable(IDB* db) {
        const auto sql =
            // int
            "CREATE TABLE `value_int` ("
            "`id` INTEGER PRIMARY KEY,"
            "`obj_id` int NOT NULL,"
            "`prop_id` int NOT NULL,"
            "`value` int,"
            "FOREIGN KEY (obj_id) REFERENCES object(id),"
            "FOREIGN KEY (prop_id) REFERENCES property(id)"
            ");"
            // double
            "CREATE TABLE `value_double` ("
            "`id` INTEGER PRIMARY KEY,"
            "`obj_id` int NOT NULL,"
            "`prop_id` int NOT NULL,"
            "`value` DOUBLE,"
            "FOREIGN KEY (obj_id) REFERENCES object(id),"
            "FOREIGN KEY (prop_id) REFERENCES property(id)"
            ");"
            // string
            "CREATE TABLE `value_string` ("
            "`id` INTEGER PRIMARY KEY,"
            "`obj_id` int NOT NULL,"
            "`prop_id` int NOT NULL,"
            "`value` varchar(255),"
            "FOREIGN KEY (obj_id) REFERENCES object(id),"
            "FOREIGN KEY (prop_id) REFERENCES property(id)"
            ");";

        db->execute(sql, {});
    }

    void DBInitializer::createTablesAndFKeys(IDB* db) {
        db->begin();
        createCollectionTable(db);
        createObjectTable(db);
        createPropertyTable(db);
        createValuesTable(db);
        db->commit();
    }
}  // namespace nldb