#include <gtest/gtest.h>

#include "Enums.hpp"
#include "PropertyRep.hpp"
#include "Query.hpp"
#include "SqlStatement.hpp"

TEST(PropertyTests, ShouldGetSameNames) {
    auto model = PropertyRep("model", -1, PropertyType::STRING);

    EXPECT_EQ(model.getName(), "model");
    EXPECT_EQ(model.getStatement(), "year");
    EXPECT_EQ(model.getId(), -1);
}