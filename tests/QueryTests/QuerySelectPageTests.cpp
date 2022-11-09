#include <gtest/gtest.h>

#include "QueryBaseCars.hpp"
#include "nldb/Collection.hpp"

TYPED_TEST(QueryCarsTest, ShouldSelectByPage) {
    const int nCars = this->data_cars.size();

    json results[nCars];

    for (int i = 0; i < nCars; i++) {
        json res = this->q.from("cars").select().page(i + 1, 1).execute();

        ASSERT_EQ(res.size(), 1) << res;
        results[i] = res[0];
    }

    for (int i = 0; i < nCars - 1; i++) {
        ASSERT_NE(results[i], results[i + 1]);
    }
}

TYPED_TEST(QueryCarsTest, ShouldSelectByPageOrdered) {
    const int nCars = this->data_cars.size();

    Collection cars = this->q.collection("cars");

    json results[nCars];

    for (int i = 0; i < nCars; i++) {
        json res = this->q.from("cars")
                       .select()
                       .page(i + 1, 1)
                       .sortBy(cars["year"].desc())
                       .execute();

        ASSERT_EQ(res.size(), 1) << res;
        results[i] = res[0];
    }

    for (int i = 0; i < nCars - 1; i++) {
        // 0 = greater
        ASSERT_GT(results[i]["year"], results[i + 1]["year"]);
    }
}

TYPED_TEST(QueryCarsTest, ShouldSelectNoneIfNoElementsInPage) {
    Collection cars = this->q.collection("cars");
    json res1 = this->q.from("cars")
                    .select()
                    .where(cars["year"] == (int)this->data_cars[0]["year"])
                    .page(1, 1)
                    .execute();

    ASSERT_EQ(res1.size(), 1) << res1;

    json res2 = this->q.from("cars")
                    .select()
                    .where(cars["year"] == (int)this->data_cars[0]["year"])
                    .page(2, 1)
                    .execute();

    ASSERT_EQ(res2.size(), 0) << res2;
}