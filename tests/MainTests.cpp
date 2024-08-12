#include "gtest/gtest.h"
#include "nldb/LOG/log.hpp"
#include "nldb/LOG/managers/log_constants.hpp"

void signal_catcher(int sig) {
    nldb::LogManager::GetLogger()->flush();
    exit(sig);
}

int main(int argc, char** argv) {
    signal(SIGINT, signal_catcher);
    signal(SIGSEGV, signal_catcher);
    signal(SIGABRT, signal_catcher);
    signal(SIGTERM, signal_catcher);

    ::testing::InitGoogleTest(&argc, argv);
    nldb::LogManager::Initialize();
    nldb::LogManager::SetLevel(nldb::log_level::warn);

    return RUN_ALL_TESTS();
}