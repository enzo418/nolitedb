#include "QueryBase.hpp"

using namespace nldb;

template <typename T>
class QueryInsertTests : public QueryBaseTest<T> {};

TYPED_TEST_SUITE(QueryInsertTests, TestDBTypes);

TYPED_TEST(QueryInsertTests, ShouldInsertOneConstData) {
    this->q.from("test").insert({{"name", "pepe"}});

    EXPECT_GT(this->db.getChangesCount(), 0);
}

TYPED_TEST(QueryInsertTests, ShouldInsertBulkConstData) {
    this->q.from("test").insert({{{"name", "pepe"}}, {{"name", "x"}}});

    EXPECT_GT(this->db.getChangesCount(), 0);
}

TYPED_TEST(QueryInsertTests, ShouldInsertOneRefData) {
    json ob = {{{"name", "pepe"}}};
    this->q.from("test").insert(ob);

    EXPECT_GT(this->db.getChangesCount(), 0);
}

TYPED_TEST(QueryInsertTests, ShouldInsertBulkRefData) {
    json ob = {{{"name", "pepe"}}, {{"name", "x"}}};
    this->q.from("test").insert(ob);

    EXPECT_GT(this->db.getChangesCount(), 0);
}