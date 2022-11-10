#include <gtest/gtest.h>

#include "QueryBase.hpp"
#include "QueryBaseCars.hpp"
#include "nldb/Collection.hpp"
#include "nldb/Common.hpp"

using namespace nldb;

template <typename T>
class QuerySelectGroupedTestsCars : public QueryCarsTest<T> {};
TYPED_TEST_SUITE(QuerySelectGroupedTestsCars, TestDBTypes);

TYPED_TEST(QuerySelectGroupedTestsCars, ShouldSelectGroupedByMakerModel) {
    Collection cars = this->q.collection("cars");

    json result = this->q.from("cars")
                      .select(cars["model"], cars["maker"])
                      .groupBy(cars["model"], cars["maker"])
                      .execute();

    EXPECT_EQ(result.size(), 2);

    EXPECT_NE(result[0]["model"], result[1]["model"]);
    EXPECT_NE(result[0]["maker"], result[1]["maker"]);
}
