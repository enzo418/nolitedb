#include <gtest/gtest.h>

#include "QueryBaseNumbers.hpp"
#include "nldb/Collection.hpp"
#include "nldb/Common.hpp"

template <typename T>
class QuerySelectAggregatedTests : public QueryNumbersTest<T> {};

TYPED_TEST_SUITE(QuerySelectAggregatedTests, TestDBTypes);

TYPED_TEST(QuerySelectAggregatedTests, ShouldSelectMax) {
    Collection numbers = this->q.collection("numbers");
    json res = this->q.from("numbers")
                   .select(numbers["name"],
                           numbers["double_rep"].maxAs("max_double_rep"))
                   .execute();

    ASSERT_EQ(res.size(), 1);
    ASSERT_EQ(res[0]["name"], "pi");
    ASSERT_NEAR(res[0]["max_double_rep"], M_PI, 1e-2) << res;
}

TYPED_TEST(QuerySelectAggregatedTests, ShouldSelectMaxInnerObject) {
    Collection numbers = this->q.collection("numbers");
    json res =
        this->q.from("numbers")
            .select(numbers["name"],
                    numbers["extra.known_since"].maxAs("max_known_since"))
            .execute();

    ASSERT_EQ(res.size(), 1) << res;
    ASSERT_EQ(countMembers(res[0]), 2) << res;
    ASSERT_EQ(countMembers(res[0]["extra"]), 1) << res;
    ASSERT_EQ(res[0]["name"], "imaginary number") << res;
    ASSERT_EQ(res[0]["extra"]["max_known_since"], 23) << res;
}