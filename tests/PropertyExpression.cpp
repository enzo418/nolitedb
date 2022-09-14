#include <gtest/gtest.h>

#include <regex>

#include "Enums.hpp"
#include "PropertyRep.hpp"
#include "Query.hpp"
#include "SqlStatement.hpp"
#include "gtest/gtest.h"

TEST(PropertyExpression, BothAreProperty) {
    auto model = PropertyRep("model", -1, PropertyType::STRING);
    auto year = PropertyRep("year", -1, PropertyType::INTEGER);

    std::vector<SqlLogicExpression> criterias = {
        model<year, model <= year, model> year,
        model >= year,
        model == year,
        model != year,

        model % "%like%",
        model ^ "%nlike%"};

    std::vector<Operator> cds = {LT, LTE, GT, GTE, EQ, NEQ, LIKE, NLIKE};

    for (int i = 0; i < criterias.size(); i++) {
        const auto ct = criterias[i];
        auto st = ct.getStatement();

        EXPECT_TRUE(st.find(OperatorToString(cds[i])) != std::string::npos);
    }
}

TEST(PropertyExpression, RightIsConstant) {
    auto year = PropertyRep("year", -1, PropertyType::INTEGER);

    std::vector<SqlLogicExpression> criterias = {
        year<418.42, year <= 418.42, year> 418.42,
        year >= 418.42,
        year == 418.42,
        year != 418.42,
    };

    std::vector<Operator> cds = {LT, LTE, GT, GTE, EQ, NEQ};

    for (int i = 0; i < criterias.size(); i++) {
        const auto ct = criterias[i];
        auto st = ct.getStatement();

        EXPECT_TRUE(st.find(OperatorToString(cds[i])) != std::string::npos);
    }
}

TEST(PropertyExpression, ComposedLogicOperators) {
    auto model = PropertyRep("model", -1, PropertyType::STRING);
    auto year = PropertyRep("year", -1, PropertyType::INTEGER);

    auto comp = model < year && year >= 2000;
    auto comp2 = comp || model % "test%";
    auto comp3 = comp2 && year != 2007;

    auto posAND = comp.getStatement().find(OperatorToString(Operator::AND));
    auto posOR = comp.getStatement().find(OperatorToString(Operator::OR));
    auto posAND2 =
        comp.getStatement().find(OperatorToString(Operator::AND), posAND + 1);

    EXPECT_TRUE(posAND != std::string::npos);
    EXPECT_TRUE(posOR > posAND);
    EXPECT_TRUE(posAND2 > posAND);
}