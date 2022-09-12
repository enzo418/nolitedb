#include <gtest/gtest.h>

#include "PropertyRep.hpp"
#include "Query.hpp"
#include "SqlStatement.hpp"

TEST(PropertyTests, ShouldGetSameNames) {
    auto model = PropertyRep("model");

    EXPECT_EQ(model.getName(), "model");
    EXPECT_EQ(model.getStatement(), "year");
}