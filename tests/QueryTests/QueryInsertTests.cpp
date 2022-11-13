#include "QueryBase.hpp"
#include "nldb/Common.hpp"

using namespace nldb;

template <typename T>
class QueryInsertTests : public QueryBaseTest<T> {};

TYPED_TEST_SUITE(QueryInsertTests, TestDBTypes);

TYPED_TEST(QueryInsertTests, ShouldInsertOneConstData) {
    this->q.from("test").insert({{"name", "pepe"}});

    EXPECT_GT(this->db.getChangesCount(), 0);

    json selected = this->q.from("test").select().execute();

    ASSERT_EQ(selected.size(), 1);
    ASSERT_EQ(selected[0]["name"], "pepe");
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

TYPED_TEST(QueryInsertTests, ShouldInsertWithId) {
    auto id = common::internal_id_string;
    Collection test = this->q.collection("test");

    json ob = {{{id, 42}, {"name", "pepe"}}, {{id, 45}, {"name", "x"}}};
    this->q.from("test").insert(ob);

    EXPECT_GT(this->db.getChangesCount(), 0);

    json selected =
        this->q.from("test").select().sortBy(test[id].asc()).execute();

    ASSERT_EQ(selected.size(), 2);
    ASSERT_EQ(selected[0][id], "42");
    ASSERT_EQ(selected[1][id], "45");
}