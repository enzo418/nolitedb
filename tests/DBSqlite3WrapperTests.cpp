#include <gtest/gtest.h>
#include <math.h>

#include <algorithm>
#include <string>
#include <vector>

#include "Query.hpp"
#include "dbwrapper/sq3wrapper/DB.hpp"

TEST(Sqlite3Wrapper, shouldOpenDB) {
    DBSL3 db;

    EXPECT_TRUE(db.open(":memory:"));
}

TEST(Sqlite3Wrapper, shouldBeAbleToPerformBasicCRUD) {
    DBSL3 db;

    EXPECT_TRUE(db.open(":memory:"));

    db.executeOneStep("CREATE TABLE test (id INTEGER PRIMARY KEY, value text);",
                      {});

    EXPECT_EQ(db.getChangesCount(), 0);

    std::string value1 = "teststring1";
    const char* value2 = "teststring2";

    db.executeOneStep("insert into test (value) values (@value), (@value2)",
                      {{"@value", value1}, {"@value2", value2}});

    EXPECT_EQ(db.getChangesCount(), 2);
    EXPECT_EQ(db.getLastInsertedRowId(), 2);

    auto reader = db.executeReader("select id, value from test", {});

    int rowCount = 0;
    std::shared_ptr<IDBRowReader> row;
    while (reader->readRow(row)) {
        rowCount++;

        EXPECT_EQ(rowCount, row->readInt32(0));
        EXPECT_EQ("teststring" + std::to_string(rowCount), row->readString(1));
    }

    EXPECT_EQ(rowCount, 2);
}

TEST(Sqlite3Wrapper, shouldCreateAllTheNeededTablesAtStart) {
    DBSL3 db;

    EXPECT_TRUE(db.open(":memory:"));

    auto reader = db.executeReader(
        "SELECT name FROM sqlite_schema WHERE type='table' ORDER BY name;", {});

    std::vector<std::string> tablesNames = {"collection", "document",
                                            "property",   "value_double",
                                            "value_int",  "value_string"};

    std::vector<std::string> found(tablesNames.size());

    int rowCount {0};
    std::shared_ptr<IDBRowReader> row;
    while (reader->readRow(row)) {
        rowCount++;
        found.push_back(row->readString(0));
    }

    for (auto& name : tablesNames) {
        auto it = std::find(found.begin(), found.end(), name);
        EXPECT_TRUE(it != found.end());
    }

    EXPECT_EQ(tablesNames.size(), rowCount);
}

TEST(Sqlite3Wrapper, shouldInsertADocument) {
    DBSL3 db;

    EXPECT_TRUE(db.open(":memory:"));

    auto f = QueryFactory();
    auto numbersCollection = f.create(&db, "numbers");

    json number = {
        {"number_name", "pi"}, {"double_rep", M_PI}, {"integer_rep", 3}};

    numbersCollection.insert(number).execute();

    // check if the properties were inserted
    auto valinserted = db.executeAndGetFirstInt(
        "select * from value_int left join value_string left join value_double",
        {});

    if (!valinserted.has_value()) FAIL() << "Values were not inserted";

    auto [name, dr, ir] = numbersCollection.prepareProperties(
        "number_name", "double_rep", "integer_rep");

    // TODO: update test taking the first from the select
    json result = numbersCollection.select(name, dr, ir).execute();
    EXPECT_EQ(result.size(), 1);
    auto res = result[0];

    EXPECT_EQ(res["number_name"], "pi");
    EXPECT_NEAR(res["double_rep"], M_PI, 1E-5);
    EXPECT_EQ(res["integer_rep"], 3);
}

TEST(Sqlite3Wrapper, shouldInsertMultipleDocuments) {
    DBSL3 db;

    EXPECT_TRUE(db.open(":memory:"));

    auto f = QueryFactory();
    auto numbersCollection = f.create(&db, "numbers");

    json numbers = {
        {{"number_name", "pi"}, {"double_rep", M_PI}, {"integer_rep", 3}},

        {{"number_name", "e"}, {"double_rep", M_E}, {"integer_rep", 2}},

        {{"number_name", "log2(e)"},
         {"double_rep", M_LOG2E},
         {"integer_rep", 1}}};

    numbersCollection.insert(numbers).execute();

    // check if the properties were inserted
    auto valinserted = db.executeAndGetFirstInt(
        "select * from value_int left join value_string left join value_double",
        {});

    if (!valinserted.has_value()) FAIL() << "Values were not inserted";

    auto [name, dr, ir] = numbersCollection.prepareProperties(
        "number_name", "double_rep", "integer_rep");

    json result = numbersCollection.select(name, dr, ir).execute();

    EXPECT_EQ(result.size(), numbers.size());

    // TODO: Update tests after order by is added
    std::sort(result.begin(), result.end(), [](auto a, auto b) {
        return a["integer_rep"] > b["integer_rep"];
    });

    for (int i = 0; i < numbers.size(); i++) {
        auto r = result[i];
        auto n = numbers[i];

        EXPECT_EQ(r["number_name"], n["number_name"]);
        EXPECT_NEAR(r["double_rep"], n["double_rep"], 1E-5);
        EXPECT_EQ(r["integer_rep"], n["integer_rep"]);
    }
}