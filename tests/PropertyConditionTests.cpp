#include <gtest/gtest.h>

#include "PropertyRep.hpp"
#include "Query.hpp"
#include "SqlStatement.hpp"

TEST(PropertyConditionTests, ConditionsBothAreProperty) {
    // auto model = PropertyRep("model");
    // auto year = PropertyRep("year");

    // auto c = (model > year) && (year != model);

    // auto c1 = model < year;
    // auto c2 = model < 200;

    // auto c3 = c1 && c2;
    // auto c4 = c1 || c3;
    // auto c5 = ~c4;

    // std::cout << "Result: " << c5.getStatement() << std::endl;
}

TEST(PropertyConditionTests, ConditionsBothRightIsConstant) {
    // auto factory = QueryFactory();

    // auto [query, model, maker, year] =
    //     factory.properties("model", "maker", "year");

    // std::vector<PropertyCondition> criterias = {
    //     year<418.42, year <= 418.42, year> 418.42, year >= 418.42,
    //     year == 418.42, year != 418.42,
    //     // the operations below gives type traits errors :)
    //     // year % 418.42,
    //     // year ^ 418.42
    // };

    // std::vector<int> cds = {LT, LTE, GT, GTE, EQ, NEQ};

    // for (int i = 0; i < criterias.size(); i++) {
    //     const auto ct = criterias[i];
    //     auto [left, cond, right] = ct.get();

    //     EXPECT_EQ(left->getName(), year.getName());
    //     EXPECT_EQ(cond, cds[i]);
    //     EXPECT_TRUE(std::holds_alternative<double>(right));
    //     EXPECT_NEAR(std::get<double>(right), 418.42, .001);
    // }
}