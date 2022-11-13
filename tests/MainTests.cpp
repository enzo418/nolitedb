#include "gtest/gtest.h"
#include "nldb/LOG/log.hpp"

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    nldb::LogManager::Initialize();

    return RUN_ALL_TESTS();
}