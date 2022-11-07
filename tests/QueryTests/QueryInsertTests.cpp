#include "QueryBase.hpp"

using namespace nldb;

TYPED_TEST_SUITE(QueryBaseTest, TestDBTypes);

TYPED_TEST(QueryBaseTest, ShouldInsertOneConstData) {
  this->q.from("test").insert({{"name", "pepe"}});

  EXPECT_GT(this->db.getChangesCount(), 0);
}

TYPED_TEST(QueryBaseTest, ShouldInsertBulkConstData) {
  this->q.from("test").insert({{{"name", "pepe"}}, {{"name", "x"}}});

  EXPECT_GT(this->db.getChangesCount(), 0);
}

TYPED_TEST(QueryBaseTest, ShouldInsertOneRefData) {
  json ob = {{{"name", "pepe"}}};
  this->q.from("test").insert(ob);

  EXPECT_GT(this->db.getChangesCount(), 0);
}

TYPED_TEST(QueryBaseTest, ShouldInsertBulkRefData) {
  json ob = {{{"name", "pepe"}}, {{"name", "x"}}};
  this->q.from("test").insert(ob);

  EXPECT_GT(this->db.getChangesCount(), 0);
}