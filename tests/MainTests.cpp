#include "gtest/gtest.h"
#include "nldb/LOG/log.hpp"
#include "nldb/LOG/managers/log_constants.hpp"

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    nldb::LogManager::Initialize();
    nldb::LogManager::SetLevel(nldb::log_level::err);

    return RUN_ALL_TESTS();
}