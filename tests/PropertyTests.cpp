#include <gtest/gtest.h>

#include "Enums.hpp"
#include "PropertyRep.hpp"
#include "Query.hpp"
#include "SqlStatement.hpp"

TEST(PropertyTests, ShouldGetSameNames) {
    auto model = PropertyRep("model", -1, PropertyType::STRING);
    auto year = PropertyRep("year", 5, PropertyType::INTEGER);

    EXPECT_EQ(model.getName(), "model");
    EXPECT_EQ(model.getStatement(), "model_vs");
    EXPECT_EQ(model.getId(), -1);
    EXPECT_EQ(model.getType(), PropertyType::STRING);

    EXPECT_EQ(year.getName(), "year");
    EXPECT_EQ(year.getStatement(), "year_vi");
    EXPECT_EQ(year.getId(), 5);
    EXPECT_EQ(year.getType(), PropertyType::INTEGER);
}