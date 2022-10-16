#include <gtest/gtest.h>

#include "nldb/Utils/ValueBuffer.hpp"

using namespace nldb;

const int BUFFER_SIZE = 4;

class ValueBufferTest : public testing::Test {
   protected:
    ValueBufferTest() : buffer(BUFFER_SIZE) {}

    void SetUp() override {}
    void TearDown() override {}

    Buffer<int> buffer;
};

TEST_F(ValueBufferTest, SimpleAdd) {
    EXPECT_TRUE(this->buffer.Add(1));
    EXPECT_TRUE(this->buffer.Add(2));
    EXPECT_TRUE(this->buffer.Add(3));
    EXPECT_TRUE(this->buffer.Add(4));
    EXPECT_FALSE(this->buffer.Add(5));

    buffer.ForEach([](int el, bool isLast) {
        if (el < 4)
            EXPECT_FALSE(isLast);
        else if (el == 4)
            EXPECT_TRUE(isLast);
        else
            EXPECT_FALSE(true) << "this should not happen!";
    });

    buffer.Reset();

    EXPECT_TRUE(this->buffer.Add(42));
    buffer.ForEach([](int el, bool isLast) {
        EXPECT_TRUE(isLast);
        EXPECT_EQ(el, 42);
    });
}