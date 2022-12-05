#include <gtest/gtest.h>

#include "QueryBase.hpp"
#include "QueryBaseCars.hpp"
#include "nldb/Collection.hpp"
#include "nldb/Common.hpp"
#include "nldb/typedef.hpp"

using namespace nldb;

template <typename T>
class QueryUpdateTestsCars : public QueryCarsTest<T> {};
TYPED_TEST_SUITE(QueryUpdateTestsCars, TestDBTypes);

TYPED_TEST(QueryUpdateTestsCars, ShouldUpdateAnExistingField) {
    Collection cars = this->q.collection("cars");
    json newestCars =
        this->q.from("cars").select().sortBy(cars["year"].desc()).execute();

    ASSERT_GE(newestCars.size(), 1);

    const std::string newestId = newestCars[0][common::internal_id_string];

    this->q.from("cars").update(newestId, {{"year", 112233}});

    json newestCars2 =
        this->q.from("cars").select().sortBy(cars["year"].desc()).execute();

    ASSERT_GE(newestCars2.size(), 1);
    ASSERT_EQ(newestCars[0][common::internal_id_string],
              newestCars2[0][common::internal_id_string]);
    ASSERT_EQ(newestCars2[0]["year"], 112233);

    // assert that the rest wan't modified
    for (int i = 1; i < newestCars.size(); i++) {
        ASSERT_EQ(newestCars[i], newestCars2[i]);
    }
}

TYPED_TEST(QueryUpdateTestsCars, ShouldUpdateMultipleExistingField) {
    Collection cars = this->q.collection("cars");
    json newestCars =
        this->q.from("cars").select().sortBy(cars["year"].desc()).execute();

    ASSERT_GE(newestCars.size(), 1);

    const std::string newestId = newestCars[0][common::internal_id_string];

    json lUpdateValue = {{"maker", "not ford"}, {"categories", {-1, 0}}};

    this->q.from("cars").update(newestId, lUpdateValue);

    json newestCars2 =
        this->q.from("cars").select().sortBy(cars["year"].desc()).execute();

    ASSERT_GE(newestCars2.size(), 1);
    ASSERT_EQ(newestCars[0][common::internal_id_string],
              newestCars2[0][common::internal_id_string]);
    ASSERT_EQ(newestCars2[0]["maker"], "not ford");
    ASSERT_EQ(newestCars2[0]["categories"], lUpdateValue["categories"]);

    for (int i = 1; i < newestCars.size(); i++) {
        ASSERT_EQ(newestCars[i], newestCars2[i]);
    }
}

TYPED_TEST(QueryUpdateTestsCars, ShouldSetNonExistingField) {
    Collection cars = this->q.collection("cars");
    json newestCars =
        this->q.from("cars").select().sortBy(cars["year"].desc()).execute();

    ASSERT_GE(newestCars.size(), 1);

    snowflake id = std::stoll(
        newestCars[0][common::internal_id_string].get<std::string>());

    this->q.from("cars").update(
        id, {{"test_field", "test_value"}, {"maker", "xx"}});

    json modNewestCars =
        this->q.from("cars").select().sortBy(cars["year"].desc()).execute();

    ASSERT_GE(modNewestCars.size(), 1);

    ASSERT_FALSE(newestCars[0].contains("test_field"));
    ASSERT_TRUE(modNewestCars[0].contains("test_field"));
    ASSERT_EQ(modNewestCars[0]["test_field"], "test_value");

    ASSERT_NE(newestCars[0]["maker"], modNewestCars[0]["maker"]);
    ASSERT_EQ(modNewestCars[0]["maker"], "xx");

    for (int i = 1; i < newestCars.size(); i++) {
        ASSERT_EQ(newestCars[i], modNewestCars[i]);
    }
}

TYPED_TEST(QueryUpdateTestsCars, ShouldUpdateAndSetInnerField) {
    Collection cars = this->q.collection("cars");
    json newestCars =
        this->q.from("cars").select().where(cars["year"] == 2011).execute();

    const std::string id =
        newestCars[0][common::internal_id_string].get<std::string>();

    this->q.from("cars").update(id, {{"technical",
                                      {{"length", 418},
                                       {"weight", "0.1 kg"},
                                       /*new field speed*/ {"speed", 3.5}}}});

    json updated =
        this->q.from("cars").select().where(cars["year"] == 2011).execute();

    ASSERT_EQ(updated.size(), 1);

    ASSERT_TRUE(updated[0].contains("technical"));
    ASSERT_TRUE(updated[0]["technical"].contains("length"));
    ASSERT_TRUE(updated[0]["technical"].contains("weight"));
    ASSERT_TRUE(updated[0]["technical"].contains("speed"));

    ASSERT_EQ(updated[0]["technical"]["length"], 418);
    ASSERT_EQ(updated[0]["technical"]["weight"], "0.1 kg");
    ASSERT_NEAR(updated[0]["technical"]["speed"], 3.5, 1e-2);
}

TYPED_TEST(QueryUpdateTestsCars, ShouldSetNonExistingInnerField) {
    Collection cars = this->q.collection("cars");
    json newestCars =
        this->q.from("cars").select().where(cars["year"] == 2015).execute();

    std::string id =
        newestCars[0][common::internal_id_string].get<std::string>();

    json newFields = {
        //
        {"logs",
         {{"gyro", {{"acc", "99 m/s2"}}}, {"latency", {1, 1, 1, 10, 0}}}},
        {"pressure", {{"fl", 1.0}, {"fr", 0.5}}}
        //
    };

    this->q.from("cars").update(id, newFields);

    json updated = this->q.from("cars")
                       .select()
                       .where(cars[common::internal_id_string] == id)
                       .execute();

    ASSERT_EQ(updated.size(), 1);

    ASSERT_TRUE(updated[0].contains("logs"));
    ASSERT_TRUE(updated[0].contains("pressure"));

    ASSERT_TRUE(equalObjectsIgnoreID(updated[0]["logs"], newFields["logs"]));
    ASSERT_TRUE(
        equalObjectsIgnoreID(updated[0]["pressure"], newFields["pressure"]));
}

TYPED_TEST(QueryUpdateTestsCars, ShouldNotThrowOnIntegerIntoDouble) {
    Collection cars = this->q.collection("cars");
    json car =
        this->q.from("cars").select().where(cars["year"] == 2003).execute();

    ASSERT_GE(car.size(), 1);

    const std::string id = car[0][common::internal_id_string];

    EXPECT_NO_THROW(
        this->q.from("cars").update(id, {{"technical", {{"0-60 mph", 99}}}}));

    std::cout << this->q.from("cars")
                     .select()
                     .where(cars[common::internal_id_string] == id)
                     .execute()[0]
                     .dump(2)
              << std::endl;

    EXPECT_NEAR(this->q.from("cars")
                    .select()
                    .where(cars[common::internal_id_string] == id)
                    .execute()[0]["technical"]["0-60 mph"],
                99, 1e-2);
}

TYPED_TEST(QueryUpdateTestsCars, ShouldMaybeNotThrowOnIntegerIntoDouble) {
    Collection cars = this->q.collection("cars");
    json car =
        this->q.from("cars").select().where(cars["year"] == 2003).execute();

    ASSERT_GE(car.size(), 1);

    const std::string id = car[0][common::internal_id_string];

#if NLDB_ENABLE_DOUBLE_DOWNCASTING
    EXPECT_NO_THROW(this->q.from("cars").update(id, {{"year", 2007.02}}));

    EXPECT_EQ(this->q.from("cars")
                  .select()
                  .where(cars[common::internal_id_string] == id)
                  .execute()[0]["year"],
              2007);
#else
    // EXPECT_THROW(..., WrongPropertyType) // pure virtual method???????
    try {
        this->q.from("cars").update(id, {{"year", 2003.02}});
    } catch (const WrongPropertyType& t) {
        EXPECT_EQ(t.expected, "INTEGER");
        EXPECT_EQ(t.actual, "DOUBLE");
    } catch (...) {
        ADD_FAILURE() << "Unexpected exception thrown";
    }
#endif
}