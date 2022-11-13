#include <gtest/gtest.h>

#include "QueryBaseCars.hpp"
#include "QueryBaseNumbers.hpp"
#include "nldb/Collection.hpp"
#include "nldb/Common.hpp"

template <typename T>
class QuerySelectAggregatedTests : public QueryNumbersTest<T> {};

TYPED_TEST_SUITE(QuerySelectAggregatedTests, TestDBTypes);

template <typename T>
class QuerySelectAggregatedTestsCars : public QueryCarsTest<T> {};
TYPED_TEST_SUITE(QuerySelectAggregatedTestsCars, TestDBTypes);

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
                    numbers["extra"]["known_since"].maxAs("max_known_since"))
            .execute();

    ASSERT_EQ(res.size(), 1) << res;
    ASSERT_EQ(countMembers(res[0]), 2) << res;
    ASSERT_EQ(countMembers(res[0]["extra"]), 1) << res;
    ASSERT_EQ(res[0]["name"], "imaginary number") << res;
    ASSERT_EQ(res[0]["extra"]["max_known_since"], 23) << res;
}

TYPED_TEST(QuerySelectAggregatedTests, ShouldSelectMaxEmbedDocument) {
    Collection numbers = this->q.collection("numbers");
    Collection numbers_usage = this->q.collection("numbers_usage");

    json res =
        this->q.from("numbers")
            .select(numbers["name"], numbers_usage["used"].maxAs("times_used"))
            .where(numbers["name"] == numbers_usage["name"])
            .execute();

    ASSERT_EQ(res.size(), 1) << res;
    ASSERT_EQ(countMembers(res[0]), 2) << res;
    ASSERT_EQ(countMembers(res[0]["numbers_usage"]), 1) << res;
    ASSERT_EQ(res[0]["name"], "pi") << res;
    ASSERT_EQ(res[0]["numbers_usage"]["times_used"], 100) << res;
}

TYPED_TEST(QuerySelectAggregatedTests, ShouldSelectMin) {
    Collection numbers = this->q.collection("numbers");

    json res = this->q.from("numbers")
                   .select(numbers["name"],
                           numbers["double_rep"].minAs("min_double_rep"))
                   .execute();

    ASSERT_EQ(res.size(), 1);
    ASSERT_EQ(res[0]["name"], "imaginary number");
    ASSERT_EQ(res[0]["min_double_rep"], -1) << res;
}

TYPED_TEST(QuerySelectAggregatedTests, ShouldSelectSum) {
    Collection numbers = this->q.collection("numbers");

    json res = this->q.from("numbers")
                   .select(numbers["integer_rep"].sumAs("total_integer_rep"))
                   .execute();

    int total = 0;
    for (json& doc : this->data_numbers) {
        total += doc["integer_rep"].get<int>();
    }

    ASSERT_EQ(res.size(), 1);
    ASSERT_EQ(res[0]["total_integer_rep"], total) << res;
}

TYPED_TEST(QuerySelectAggregatedTests, ShouldSelectAverage) {
    Collection numbers = this->q.collection("numbers");

    json res = this->q.from("numbers")
                   .select(numbers["double_rep"].averageAs("avg_double_rep"))
                   .execute();

    double total = 0;
    for (json& doc : this->data_numbers) {
        total += doc["double_rep"].get<double>();
    }

    ASSERT_EQ(res.size(), 1);
    ASSERT_NEAR(res[0]["avg_double_rep"],
                total / (double)this->data_numbers.size(), 1e-2)
        << res;
}

TYPED_TEST(QuerySelectAggregatedTests, ShouldSelectCount) {
    Collection numbers = this->q.collection("numbers");

    // all the numbers has a name so it should be 3
    json res = this->q.from("numbers")
                   .select(numbers["name"].countAs("numbers_count"))
                   .execute();

    ASSERT_EQ(res.size(), 1);
    ASSERT_EQ(countMembers(res[0]), 1);

    // aggregate functions always set the read value to a string
    ASSERT_EQ(res[0]["numbers_count"],
              std::to_string(this->data_numbers.size()));
}

TYPED_TEST(QuerySelectAggregatedTestsCars, ShouldSelectCountWithNull) {
    Collection cars = this->q.collection("cars");

    // only 1 car has the weight field != null
    json res =
        this->q.from("cars")
            .select(cars["technical"]["weight"].countAs("cars_with_weight"))
            .execute();

    ASSERT_EQ(res.size(), 1);
    ASSERT_EQ(countMembers(res[0]), 1);
    ASSERT_EQ(res[0]["technical"]["cars_with_weight"], "1");
}
