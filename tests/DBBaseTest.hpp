#pragma once

#include "gtest/gtest.h"
#include "nldb/Query/Query.hpp"
#include "nldb/SQL3Implementation.hpp"
#include "nldb/backends/sqlite3/DB/DB.hpp"

using namespace nldb;

// declare all the types that are going to be tested
using TestDBTypes = ::testing::Types<DBSL3>;

template <typename T>
class BaseDBTest : public ::testing::Test,
                   public testing::WithParamInterface<T> {
   public:
    virtual void SetUp() override { ASSERT_TRUE(db.open(":memory:")); }

    virtual void TearDown() override { db.close(); }

    T db;
};