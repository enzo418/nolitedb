#include <gtest/gtest.h>

#include <string>

#include "QueryBase.hpp"
#include "QueryBaseCars.hpp"
#include "nldb/Collection.hpp"
#include "nldb/Common.hpp"
#include "nldb/Exceptions.hpp"
#include "nldb/Property/Property.hpp"
#include "nldb/nldb_json.hpp"

using namespace nldb;

namespace {
    // there is no need to do this in a real usage, you can use _id but i need
    // to in case you change it to ==id!!$$::  :)
    const char* idStr = common::internal_id_string;
}  // namespace

template <typename T>
class QuerySelectRenameTestsCars : public QueryCarsTest<T> {};
TYPED_TEST_SUITE(QuerySelectRenameTestsCars, TestDBTypes);

TYPED_TEST(QuerySelectRenameTestsCars, ShouldSelectAllWithNewNames) {
    Collection cars = this->q.collection("cars");

    json result = this->q.from("cars")
                      .select()
                      .rename(cars[common::internal_id_string], "carID")
                      .rename(cars["maker"], "carMaker")
                      .rename(cars["model"], "carModel")
                      .rename(cars["year"], "carYear")
                      .rename(cars["categories"], "carCategories")
                      .rename(cars["technical"], "carTechnical")
                      .rename(cars["technical"]["length"], "carTechnicalLength")
                      .execute();

    std::cout << "result: " << result.dump(2) << std::endl;

    ASSERT_TRUE(result.is_array());
    ASSERT_EQ(result.size(), this->data_cars.size());

    for (auto& car : result) {
        ASSERT_TRUE(car.contains("carMaker"));
        ASSERT_TRUE(car.contains("carModel"));
        ASSERT_TRUE(car.contains("carYear"));
        ASSERT_TRUE(car.contains("carCategories"));

        ASSERT_TRUE(car.contains("carTechnical"));
        ASSERT_FALSE(car["carTechnical"].contains("length"));

        ASSERT_TRUE(car.contains("carID"));
        ASSERT_FALSE(car.contains(common::internal_id_string));
    }

    // check that we didn't miss any
    for (auto& data_car : this->data_cars) {
        json* found = 0;

        for (auto& car : result) {
            if (car["carModel"] == data_car["model"] &&
                car["carMaker"] == data_car["maker"] &&
                car["carYear"] == data_car["year"]) {
                found = &car;
                break;
            }
        }

        if (!found) {
            ADD_FAILURE() << "missing a car: " << data_car;
        }
    }
}

TYPED_TEST(QuerySelectRenameTestsCars, ShouldRenameInnerWithCondition) {
    const std::vector<std::string> ids = this->q.from("cars").insert(
        {{"maker", "me"},
         {"technical", {{"one", {{"inner", {{"onemore", 418}}}}}}}});

    Collection cars = this->q.collection("cars");

    auto [technical] = cars.get("technical");

    json result =
        this->q.from("cars")
            .select(technical)
            .rename(technical, "carTech")
            .rename(technical["one"], "carTechOne")
            .rename(technical["one"]["inner"], "carTechOneInner")
            .rename(technical["one"]["inner"]["onemore"], "some_string")
            .where(cars[common::internal_id_string] == ids[0])
            .execute();

    ASSERT_EQ(result.size(), 1);

    result = result[0];

    ASSERT_TRUE(result.contains("carTech"));
    ASSERT_FALSE(result.contains("technical"));

    ASSERT_TRUE(result["carTech"].contains("carTechOne"));
    ASSERT_FALSE(result["carTech"].contains("one"));

    ASSERT_TRUE(result["carTech"]["carTechOne"].contains("carTechOneInner"));
    ASSERT_FALSE(result["carTech"]["carTechOne"].contains("inner"));

    ASSERT_TRUE(result["carTech"]["carTechOne"]["carTechOneInner"].contains(
        "some_string"));
    ASSERT_FALSE(
        result["carTech"]["carTechOne"]["carTechOneInner"].contains("onemore"));

    ASSERT_EQ(result["carTech"]["carTechOne"]["carTechOneInner"]["some_string"],
              418);
}
