#include <gtest/gtest.h>

#include "QueryBase.hpp"
#include "nldb/Collection.hpp"
#include "nldb/Common.hpp"
#include "nldb/Exceptions.hpp"

using namespace nldb;

template <typename T>
class QueryAnyMultithreadTests : public QueryBaseTest<T> {};

TYPED_TEST_SUITE(QueryAnyMultithreadTests, TestDBTypes);

TYPED_TEST(QueryAnyMultithreadTests, ShouldHandleMultiThreadedOperations) {
    constexpr int inserts = 10;
    constexpr int runs = 10;

    Collection cars = this->q.collection("cars");

    auto addCars = [&](int run) {
        for (int i = 0; i < inserts; ++i) {
            json car = {
                {"maker",
                 "maker-" + std::to_string(i) + "-" + std::to_string(run)},
                {"year", 2030},
                {"technical", {{"weight", 1000 + i}, {"length", 4 + (i % 3)}}}};
            this->q.from(cars).insert(car);
        }

        // for (int j = 0; j < inserts; ++j) {
        //     json res =
        //         this->q.from(cars)
        //             .select()
        //             .where(cars["maker"] == "maker-" + std::to_string(j) +
        //             "-" +
        //                                         std::to_string(run))
        //             .execute();

        //     ASSERT_TRUE(res.is_array());
        //     ASSERT_EQ(res.size(), inserts);
        //     for (auto& car : res) {
        //         std::string id =
        //             car[common::internal_id_string].get<std::string>();
        //         json newFields = {{"year", 2030}};
        //         this->q.from(cars).update(id, newFields);
        //     }
        // }
    };

    auto queryCars = [&]() {
        for (int i = 0; i < inserts; ++i) {
            try {
                json result = this->q.from(cars)
                                  .select(cars["maker"], cars["year"],
                                          cars["technical"]["weight"])
                                  .where(cars["year"] == 2030)
                                  .execute();

                ASSERT_TRUE(result.is_array());
                for (auto& car_res : result) {
                    ASSERT_EQ(car_res["year"], 2030);
                    ASSERT_TRUE(car_res.contains("maker"));
                    ASSERT_TRUE(car_res.contains("technical"));
                    ASSERT_TRUE(car_res["technical"].contains("weight"));
                }
            } catch (nldb::CollectionNotFound& e) {
                // expected
            }
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < runs; ++i) {
        threads.emplace_back(addCars, i + 1);
        threads.emplace_back(queryCars);
    }

    for (auto& t : threads) {
        t.join();
    }

    // Final verification
    json finalResult =
        this->q.from(cars).select().where(cars["year"] == 2030).execute();

    ASSERT_TRUE(finalResult.is_array());
    ASSERT_EQ(finalResult.size(), runs * inserts);
    for (auto& car_res : finalResult) {
        ASSERT_EQ(car_res["year"], 2030);
        ASSERT_TRUE(car_res.contains("maker"));
        ASSERT_TRUE(car_res.contains("technical"));
        ASSERT_TRUE(car_res["technical"].contains("weight"));
    }
}