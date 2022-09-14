#include <gtest/gtest.h>

#include "dbwrapper/ParamsBind.hpp"

TEST(ParamsBindsTests, shouldBeEmptyAtStart) {
    Paramsbind bind;
    EXPECT_TRUE(bind.size() == 0);
}

TEST(ParamsBindsTests, shouldParseSQL) {
    Paramsbind bind = {{"@tab_name", "test"}, {"@id", 1}};
    const auto sql = "select * from @tab_name where id = @id";

    const auto parsed = utils::paramsbind::parseSQL(sql, bind);

    EXPECT_EQ(parsed, "select * from 'test' where id = 1");
}

TEST(ParamsBindsTests, shouldParseInjectedSQL) {
    Paramsbind bind = {{"@tab_name", "test"}, {"@id", "1 OR 1=1"}};
    const auto sql = "select * from @tab_name where id = @id";

    const auto parsed = utils::paramsbind::parseSQL(sql, bind);

    EXPECT_TRUE(parsed == "select * from 'test' where id = \'1 OR 1=1\'" ||
                parsed == "select * from 'test' where id = \"1 OR 1=1\"");
}