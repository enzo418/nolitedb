#include "dbwrapper/sq3wrapper/DBInitializer.hpp"

#include "logger/Logger.h"

void createCollectionTable(IDB* db) {
    const auto sql =
        "CREATE TABLE `collection` ("
        "`id` INTEGER PRIMARY KEY,"
        "`name` varchar(255)"
        ")";

    db->executeMultipleOnOneStepRaw(sql, {});
}

void createDocumentTable(IDB* db) {
    const auto sql =
        "CREATE TABLE `document` ("
        "`id` INTEGER PRIMARY KEY,"
        "`coll_id` int,"
        "FOREIGN KEY (coll_id) REFERENCES collection(id)"
        ")";

    db->executeMultipleOnOneStepRaw(sql, {});
}

void createPropertyTable(IDB* db) {
    const auto sql =
        "CREATE TABLE `property` ("
        "`id` INTEGER PRIMARY KEY,"
        "`coll_id` int,"
        "`name` varchar(255),"
        "`type` int,"
        "FOREIGN KEY (coll_id) REFERENCES collection(id)"
        ")";

    db->executeMultipleOnOneStepRaw(sql, {});
}

void createValuesTable(IDB* db) {
    const auto sql =
        // int
        "CREATE TABLE `value_int` ("
        "`id` INTEGER PRIMARY KEY,"
        "`doc_id` int,"
        "`prop_id` int,"
        "`value` int,"
        "FOREIGN KEY (doc_id) REFERENCES document(id),"
        "FOREIGN KEY (prop_id) REFERENCES property(id)"
        ");"
        // double
        "CREATE TABLE `value_double` ("
        "`id` INTEGER PRIMARY KEY,"
        "`doc_id` int,"
        "`prop_id` int,"
        "`value` DOUBLE,"
        "FOREIGN KEY (doc_id) REFERENCES document(id),"
        "FOREIGN KEY (prop_id) REFERENCES property(id)"
        ");"
        // string
        "CREATE TABLE `value_string` ("
        "`id` INTEGER PRIMARY KEY,"
        "`doc_id` int,"
        "`prop_id` int,"
        "`value` varchar(255),"
        "FOREIGN KEY (doc_id) REFERENCES document(id),"
        "FOREIGN KEY (prop_id) REFERENCES property(id)"
        ")";

    db->executeMultipleOnOneStepRaw(sql, {});
}

void DBInitializer::createTablesAndFKeys(IDB* db) {
    createCollectionTable(db);
    createDocumentTable(db);
    createPropertyTable(db);
    createValuesTable(db);
}