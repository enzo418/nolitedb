#include <gtest/gtest.h>

#include "QueryBase.hpp"
#include "QueryBaseCars.hpp"
#include "nldb/Collection.hpp"
#include "nldb/Common.hpp"
#include "nldb/Exceptions.hpp"
#include "nldb/Property/Property.hpp"
#include "nldb/nldb_json.hpp"

using namespace nldb;

// there is no need to do this in a real usage, you can use _id but i need to in
// case you change it to ==id!!$$::  :)
const char* idStr = common::internal_id_string;

template <typename T>
class QuerySelectTestsCars : public QueryCarsTest<T> {};
TYPED_TEST_SUITE(QuerySelectTestsCars, TestDBTypes);

TYPED_TEST(QuerySelectTestsCars, ShouldSelectAll) {
    json result = this->q.from("cars").select().execute();

    ASSERT_TRUE(result.is_array());
    ASSERT_EQ(result.size(), this->data_cars.size());

    for (auto& car : result) {
        ASSERT_TRUE(car.contains("maker"));
        ASSERT_TRUE(car.contains("model"));
        ASSERT_TRUE(car.contains("year"));
        ASSERT_TRUE(car.contains("categories"));
        ASSERT_TRUE(car.contains("technical"));
        ASSERT_TRUE(car.contains(common::internal_id_string));
    }

    // check if it has all the properties with the same value
    for (auto& data_car : this->data_cars) {
        json* found = 0;

        for (auto& car : result) {
            if (car["model"] == data_car["model"] &&
                car["maker"] == data_car["maker"] &&
                car["year"] == data_car["year"]) {
                found = &car;
                break;
            }
        }

        if (!found) {
            ADD_FAILURE() << "missing a car: " << data_car;
        }

        json& car = *found;
        for (auto& [k, v] : data_car.items()) {
            ASSERT_TRUE(equalObjectsIgnoreID(car, data_car));
        }
    }
}

TYPED_TEST(QuerySelectTestsCars, ShouldSelectSome) {
    Collection automaker = this->q.collection("automaker");
    json result = this->q.from("automaker")
                      .select(automaker[idStr], automaker["name"])
                      .execute();

    ASSERT_TRUE(result.is_array());
    ASSERT_EQ(result.size(), this->data_automaker.size());

    for (auto& maker : this->data_automaker) {
        for (auto& result_maker : result) {
            ASSERT_TRUE(result_maker.contains(idStr));
            ASSERT_TRUE(result_maker.contains("name"));
            ASSERT_EQ(countMembers(result_maker), 2);
        }
    }
}

TYPED_TEST(QuerySelectTestsCars, ShouldIgnoreSome) {
    auto [id] = this->q.collection("automaker").get(idStr);
    json result = this->q.from("automaker").select().suppress(id).execute();

    ASSERT_EQ(result.size(), this->data_automaker.size());

    for (auto& maker : this->data_automaker) {
        json* found;

        for (auto& result_maker : result) {
            if (result_maker["name"] == maker["name"]) {
                found = &result_maker;
            }
        }

        if (!found) {
            ADD_FAILURE() << "missing automaker";
        }

        ASSERT_EQ(*found, maker);
    }
}

TYPED_TEST(QuerySelectTestsCars, ShouldSelectEmbedDocuments) {
    Collection automaker = this->q.collection("automaker");
    Collection cars = this->q.collection("cars");

    // selects all the members from cars and automaker, resulting in
    // {maker, model, year, categories, technical, automaker{name, founded,
    // country}}
    json result = this->q.from(cars)
                      .select(cars, automaker)
                      .where(automaker["name"] == cars["maker"])
                      .includeInnerIds()
                      .execute();

    ASSERT_EQ(result.size(), 3) << result;

    for (auto& car : result) {
        ASSERT_TRUE(car.contains(idStr));
        ASSERT_TRUE(car.contains("maker"));
        ASSERT_TRUE(car.contains("model"));
        ASSERT_TRUE(car.contains("year"));
        ASSERT_TRUE(car.contains("categories"));

        ASSERT_TRUE(car.contains("technical"));
        ASSERT_TRUE(car["technical"].contains(idStr));

        ASSERT_TRUE(car.contains("automaker"));
        ASSERT_TRUE(car["automaker"].contains(idStr));
        ASSERT_TRUE(car["automaker"].contains("name"));
        ASSERT_TRUE(car["automaker"].contains("founded"));
        ASSERT_TRUE(car["automaker"].contains("country"));
    }

    for (auto& car : this->data_cars) {
        json* static_automaker = 0;
        for (auto& data_maker : this->data_automaker) {
            if (data_maker["name"] == car["maker"]) {
                static_automaker = &data_maker;
            }
        }

        assert(static_automaker);

        json* res_car = 0;

        for (auto& result_car : result) {
            if (result_car["year"] == car["year"]) {
                res_car = &result_car;
            }
        }

        if (!res_car) {
            ADD_FAILURE() << "missing car";
        }

        // compare the result automaker to data_automaker
        ASSERT_TRUE(
            equalObjectsIgnoreID(*static_automaker, (*res_car)["automaker"]));

        // compare the car data
        ASSERT_TRUE(equalObjectsIgnoreID(car, *res_car, {"automaker"}));
    }
}

TYPED_TEST(QuerySelectTestsCars, ShouldSelectAndSuppressEmbedDocumentMembers) {
    Collection automaker = this->q.collection("automaker");
    Collection cars = this->q.collection("cars");

    // Select should look like [{maker: "ford"}, automaker: {name: "ford"}, ...]
    // we always get all the objects that are not form the source (from)
    // collection as object to avoid name collisions
    json result =
        this->q.from(cars)
            .select(cars["maker"], automaker["name"])
            .where(automaker["name"] == cars["maker"])
            .suppress(
                automaker[common::internal_id_string])  // should not matter
            .execute();

    ASSERT_EQ(result.size(), 3) << result;

    for (auto& car_res : result) {
        ASSERT_TRUE(car_res.contains("maker")) << car_res;
        ASSERT_TRUE(car_res.contains("automaker")) << car_res;
        ASSERT_TRUE(car_res["automaker"].contains("name")) << car_res;

        ASSERT_EQ(countMembers(car_res), 2) << car_res;
        ASSERT_EQ(countMembers(car_res["automaker"]), 1)
            << car_res["automaker"];
        ASSERT_EQ(car_res["maker"], car_res["automaker"]["name"]);
    }
}

TYPED_TEST(QuerySelectTestsCars, ShouldSelectInnerObjectMember) {
    Collection cars = this->q.collection("cars");

    json result =
        this->q.from(cars).select(cars["technical"]["weight"]).execute();

    // there is only 1 car that has technical.weight
    ASSERT_EQ(result.size(), 1) << result;

    for (auto& car_res : result) {
        ASSERT_TRUE(car_res.contains("technical")) << car_res;
        ASSERT_TRUE(car_res["technical"].contains("weight")) << car_res;

        ASSERT_EQ(countMembers(car_res), 1) << car_res;
        ASSERT_EQ(countMembers(car_res["technical"]), 1) << car_res;
    }
}

TYPED_TEST(QuerySelectTestsCars, ShouldSelectEmptyOnNonExistingRootCollection) {
    json result = this->q.from("non existing collection").select().execute();

    ASSERT_TRUE(result.is_array()) << result;
    ASSERT_EQ(result.size(), 0) << result;
}

TYPED_TEST(QuerySelectTestsCars, ShouldThrowOnMissingIfConfigured) {
    QueryConfiguration defaultCfg;

    defaultCfg.ThrowOnSelectMissingProperty = true;

    auto newQ = Query(&this->db, defaultCfg);

    Collection cars = this->q.collection("cars");

    ASSERT_THROW(newQ.from("cars")
                     .select(cars["non existing field"])
                     .where(cars["non existing field 2"] != 2)
                     .execute(),
                 PropertyNotFound);
}

TYPED_TEST(QuerySelectTestsCars, ShouldNotThrowOnMissingIfConfigured) {
    QueryConfiguration defaultCfg;

    defaultCfg.ThrowOnSelectMissingProperty = false;

    auto newQ = Query(&this->db, defaultCfg);

    Collection cars = this->q.collection("cars");

    nldb::json result;
    ASSERT_NO_THROW({
        result = newQ.from("cars")
                     .select(cars["non existing field"])
                     .where(cars["non existing field 2"] != 33)
                     .execute();
    });

    ASSERT_TRUE(result.empty());
}

TYPED_TEST(QuerySelectTestsCars, ShouldSelectNormallyOnMissingIfConfigured) {
    QueryConfiguration defaultCfg;

    defaultCfg.ThrowOnSelectMissingProperty = false;

    auto newQ = Query(&this->db, defaultCfg);

    Collection cars = this->q.collection("cars");

    nldb::json result;
    ASSERT_NO_THROW({
        // expected behaviour:
        //  - delete cars["non existing field"] entry from select
        //  - simplify where condition to cars["year"] == 2011

        result = newQ.from("cars")
                     .select(cars["non existing field"], cars["year"],
                             cars["technical"]["length"])
                     .groupBy(cars["maker"])
                     .where(cars["non existing field 2"] != 33 &&
                            cars["year"] == 2011)
                     .execute();
    });

    nldb::json expectedResult =
        this->q.from("cars")
            .select(cars["year"], cars["technical"]["length"])
            .groupBy(cars["maker"])
            .where(cars["year"] == 2011)
            .execute();

    ASSERT_EQ(result.size(), 1);
    ASSERT_EQ(expectedResult.size(), 1);

    ASSERT_EQ(result[0], expectedResult[0]);

    ASSERT_EQ(result[0]["year"], 2011);

    ASSERT_TRUE(result[0].contains("technical"));
    ASSERT_EQ(result[0]["technical"]["length"], 4);
}

TYPED_TEST(QuerySelectTestsCars,
           ShouldSelectWithEmptyWhereOnMissingIfConfigured) {
    QueryConfiguration defaultCfg;

    defaultCfg.ThrowOnSelectMissingProperty = false;

    auto newQ = Query(&this->db, defaultCfg);

    Collection cars = this->q.collection("cars");

    nldb::json result;
    ASSERT_NO_THROW({
        result = newQ.from("cars")
                     .select(cars["technical"], cars["some missing field"])
                     .groupBy(cars["maker"])
                     .where(cars["non existing field 2"] != 33 &&
                                cars["non existing field 2"] == 2011 ||
                            cars["non existing field 99"] == "test")
                     .limit(1)
                     .execute();
    });

    nldb::json expectedResult = this->q.from("cars")
                                    .select(cars["technical"])
                                    .groupBy(cars["maker"])
                                    // .where()
                                    .sortBy(cars["year"].desc())
                                    .limit(1)
                                    .execute();

    ASSERT_EQ(result.size(), 1);
    ASSERT_EQ(expectedResult.size(), 1);

    ASSERT_TRUE(result[0].contains("technical"));

    ASSERT_FALSE(result[0]["technical"].empty());

    ASSERT_EQ(result[0], expectedResult[0]);
}