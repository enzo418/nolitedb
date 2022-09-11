#include <gtest/gtest.h>

#include "Query.hpp"

// Demonstrate some basic assertions.
TEST(PropertyConditionTests, BasicOperations) {
    auto factory = QueryFactory();

    auto [query, model, maker, year] =
        factory.properties("model", "maker", "year");

    std::cout << model.getName() << std::endl;

    auto criterias = {model<year, model <= year, model> year,
                      model >= year,
                      model == year,
                      model != year,
                      model % year,
                      model ^ year};

    auto cds = {LT, LTE, GT, GTE, EQ, NEQ, LIKE, NLIKE};

    for (auto& c : criterias) {
    }
    // Expect equality.
    EXPECT_EQ(7 * 6, 42);
}