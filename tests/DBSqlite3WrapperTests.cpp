#include <gtest/gtest.h>
#include <math.h>

#include <algorithm>
#include <string>
#include <vector>

#include "Query.hpp"
#include "dbwrapper/sq3wrapper/DB.hpp"

// TODO: Move the tests to individual files for each function
// TODO: Create an abstract tests for IDB
// TODO: Test them against different collections

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

    auto numbersCollection = QueryFactory::create(&db, "numbers");

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

    auto numbersCollection = QueryFactory::create(&db, "numbers");

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

    json result =
        numbersCollection.select(name, dr, ir).sort(ir.desc()).execute();

    EXPECT_EQ(result.size(), numbers.size());

    for (int i = 0; i < numbers.size(); i++) {
        auto r = result[i];
        auto n = numbers[i];

        EXPECT_EQ(r["number_name"], n["number_name"]);
        EXPECT_NEAR(r["double_rep"], n["double_rep"], 1E-5);
        EXPECT_EQ(r["integer_rep"], n["integer_rep"]);
    }
}

class Sql3WrapperNumbersTest : public ::testing::Test {
   protected:
    void SetUp() override {
        EXPECT_TRUE(db.open(":memory:"));

        auto numbersCollection = QueryFactory::create(&db, "numbers");

        // just a simple collection of elements with the 3 data types
        json numbers = {
            {{"number_name", "pi"}, {"double_rep", M_PI}, {"integer_rep", 3}},

            {{"number_name", "e"}, {"double_rep", M_E}, {"integer_rep", 2}},

            {{"number_name", "log2(e)"},
             {"double_rep", M_LOG2E},
             {"integer_rep", 1}}};

        numbersCollection.insert(numbers).execute();
    }

    void TearDown() override { db.close(); }

    DBSL3 db;
};

class Sql3WrapperCarsTest : public ::testing::Test {
   protected:
    void SetUp() override {
        EXPECT_TRUE(db.open(":memory:"));

        auto carsCollection = QueryFactory::create(&db, "cars");

        // just a simple collection of elements with the 3 data types
        json cars = {
            {{"maker", "ford"}, {"model", "focus"}, {"year", 2011}},
            {{"maker", "ford"}, {"model", "focus"}, {"year", 2015}},
            {{"maker", "subaru"}, {"model", "impreza"}, {"year", 2003}}};

        carsCollection.insert(cars).execute();
    }

    void TearDown() override { db.close(); }

    DBSL3 db;
};

TEST_F(Sql3WrapperCarsTest, shouldGroupElements) {
    auto carsCollection = QueryFactory::create(&db, "cars");

    auto [maker, model, year] =
        carsCollection.prepareProperties("maker", "model", "year");

    json result1 =
        carsCollection.select(maker, model).groupBy(maker, model).execute();

    json result2 = carsCollection.select(maker).groupBy(model).execute();

    EXPECT_EQ(result1.size(), 2);
    EXPECT_EQ(result2.size(), 2);
}

TEST_F(Sql3WrapperCarsTest, shouldSelectAggregateFunction) {
    auto carsCollection = QueryFactory::create(&db, "cars");

    auto [maker, model, year] =
        carsCollection.prepareProperties("maker", "model", "year");

    json newestModels = carsCollection.select(year.maxAs("year_newest"))
                            .groupBy(model)
                            .sort(year.desc())
                            .execute();

    json oldestModels = carsCollection.select(year.minAs("year_oldest"))
                            .groupBy(model)
                            .sort(year.asc())
                            .execute();

    json countPerModel = carsCollection.select(model.countAs("model_count"))
                             .groupBy(maker, model)
                             .sort(maker.asc())
                             .execute();

    json averageModelYear =
        carsCollection.select(year.averageAs("average_model_year"))
            .groupBy(maker, model)
            .sort(year.desc())
            .execute();

    json sumAllModelsYear =
        carsCollection.select(year.sumAs("total")).execute();

    EXPECT_EQ(newestModels.size(), 2);
    EXPECT_EQ(newestModels[0]["year_newest"], 2015);  // ford focus - 2015
    EXPECT_EQ(newestModels[1]["year_newest"], 2003);  // subaru impreza - 2003

    EXPECT_EQ(oldestModels.size(), 2);
    EXPECT_EQ(oldestModels[0]["year_oldest"], 2003);
    EXPECT_EQ(oldestModels[1]["year_oldest"], 2011);

    EXPECT_EQ(countPerModel.size(), 2);
    EXPECT_EQ(countPerModel[0]["model_count"], 2);
    EXPECT_EQ(countPerModel[1]["model_count"], 1);

    EXPECT_EQ(averageModelYear.size(), 2);
    EXPECT_NEAR(averageModelYear[0]["average_model_year"],
                (2011.0 + 2015.0) / 2.0, 1E-4);
    EXPECT_EQ(averageModelYear[1]["average_model_year"], 2003);

    EXPECT_EQ(sumAllModelsYear.size(), 1);
    EXPECT_EQ(sumAllModelsYear[0]["total"], 2011 + 2015 + 2003);

    // EXPECT_EQ(result2.size(), 2);
}

TEST_F(Sql3WrapperCarsTest, should_select_ByID) {
    auto carsCollection = QueryFactory::create(&db, "cars");

    auto [id, model, year] =
        carsCollection.prepareProperties("id", "model", "year");

    json newestModel = carsCollection.select(id, year.maxAs("year"))
                           .groupBy(model)
                           .sort(year.desc())
                           .page(1, 1)
                           .execute()
                           .front();

    int yearNewest = newestModel["year"];

    json newestAgain = carsCollection.select(id, year)
                           .where(id == newestModel["id"].get<int>())
                           .execute();

    EXPECT_EQ(yearNewest, 2015);

    EXPECT_EQ(newestAgain.size(), 1);
    EXPECT_EQ(newestAgain[0]["id"], newestModel["id"]);
    EXPECT_EQ(newestAgain[0]["year"], yearNewest);
}

TEST_F(Sql3WrapperCarsTest, should_selectAggregate_WithID) {
    auto carsCollection = QueryFactory::create(&db, "cars");

    auto [id, model] = carsCollection.prepareProperties("id", "model");

    json totalPerModel =
        carsCollection.select(id.countAs("id_count")).groupBy(model).execute();

    json maxID = carsCollection.select(id.maxAs("max_id")).execute().front();

    EXPECT_EQ(totalPerModel.size(), 2);
    EXPECT_EQ(totalPerModel[0]["id_count"], 2);
    EXPECT_EQ(totalPerModel[1]["id_count"], 1);

    EXPECT_EQ(maxID["max_id"], 3);
}

TEST_F(Sql3WrapperCarsTest, should_sort_byID) {
    auto carsCollection = QueryFactory::create(&db, "cars");

    auto [id] = carsCollection.prepareProperties("id");

    json sortedByID = carsCollection.select(id).sort(id.desc()).execute();

    // we have no clue in which order the DB inserted the elements, so
    // just compare the numbers
    EXPECT_EQ(sortedByID.size(), 3);
    EXPECT_EQ(sortedByID[0]["id"], 3);
    EXPECT_EQ(sortedByID[1]["id"], 2);
    EXPECT_EQ(sortedByID[2]["id"], 1);
}

TEST_F(Sql3WrapperNumbersTest, should_selectAllPropertiesByDefault) {
    /**
     * Note: This test won't since the collection-property "cache layer" is just
     * a static variable inside the collection class, so it's shared across all
     * test cases.
     * That cache uses the collection id, which normally is equal to 1 across
     * all tests, including for different ones, so it keeps returning that
     * cached data and no property gets created in the DB. That results in
     * problems when we try to query the database for all the properties from
     * that collection.
     * So i won't fixed it until the next release, v2, because a lot will
     * change.
     */

    GTEST_SKIP() << "this tests won't work until major reworks";

    auto numbersColl = QueryFactory::create(&db, "numbers");

    auto [name, double_rep, integer_rep] = numbersColl.prepareProperties(
        "number_name", "double_rep", "integer_rep");

    const std::string sql = "select id, name, type from property";

    auto reader = db.executeReader(sql, {{"@id", 1}});

    std::vector<PropertyRep> props = {PropertyRep("id", -1, PropertyType::ID)};
    std::shared_ptr<IDBRowReader> row;
    while (reader->readRow(row)) {
        props.push_back(PropertyRep(row->readString(1), row->readInt64(0),
                                    (PropertyType)row->readInt64(2)));
    }

    json all = numbersColl.select().execute();

    EXPECT_EQ(all.size(), 3);

    for (auto& number : all) {
        EXPECT_TRUE(number.is_object());
        EXPECT_TRUE(number.contains("number_name"));
        EXPECT_TRUE(number.contains("double_rep"));
        EXPECT_TRUE(number.contains("integer_rep"));
    }
}