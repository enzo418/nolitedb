#include <gtest/gtest.h>

#include "PropertyCondition.hpp"
#include "Query.hpp"

TEST(PropertyConditionTests, ConditionsBothAreProperty) {
    auto factory = QueryFactory();

    auto [query, model, maker, year] =
        factory.properties("model", "maker", "year");

    std::vector<PropertyCondition> criterias = {
        model<year, model <= year, model> year,
        model >= year,
        model == year,
        model != year,
        model % year,
        model ^ year};

    std::vector<int> cds = {LT, LTE, GT, GTE, EQ, NEQ, LIKE, NLIKE};

    for (int i = 0; i < criterias.size(); i++) {
        const auto ct = criterias[i];
        auto [left, cond, right] = ct.get();

        EXPECT_EQ(left->getName(), model.getName());
        EXPECT_EQ(cond, cds[i]);
        EXPECT_TRUE(std::holds_alternative<PropertyRep*>(right));
        EXPECT_EQ(std::get<PropertyRep*>(right)->getName(), year.getName());
    }
}

TEST(PropertyConditionTests, ConditionsBothRightIsConstant) {
    auto factory = QueryFactory();

    auto [query, model, maker, year] =
        factory.properties("model", "maker", "year");

    std::vector<PropertyCondition> criterias = {
        year<418.42, year <= 418.42, year> 418.42, year >= 418.42,
        year == 418.42, year != 418.42,
        // the operations below gives type traits errors :)
        // year % 418.42,
        // year ^ 418.42
    };

    std::vector<int> cds = {LT, LTE, GT, GTE, EQ, NEQ};

    for (int i = 0; i < criterias.size(); i++) {
        const auto ct = criterias[i];
        auto [left, cond, right] = ct.get();

        EXPECT_EQ(left->getName(), year.getName());
        EXPECT_EQ(cond, cds[i]);
        EXPECT_TRUE(std::holds_alternative<double>(right));
        EXPECT_NEAR(std::get<double>(right), 418.42, .001);
    }
}