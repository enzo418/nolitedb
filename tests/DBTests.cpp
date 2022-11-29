#include <gtest/gtest.h>

#include "DBBaseTest.hpp"
#include "nldb/backends/sqlite3/DB/DB.hpp"

using namespace nldb;

// if more databases engines are supported, then Typed Tests will
// be appropriate

template <typename T>
inline const char* getTemplateDB();

template <>
inline const char* getTemplateDB<DBSL3>() {
    return "CREATE TABLE user (id int PRIMARY KEY, name text, email text);"
           "INSERT INTO user (id, name, email) VALUES (1, 'user1', "
           "'user1@email.com');"
           "INSERT INTO user (id, name, email) VALUES (2, "
           "'user2','user2@email.com');";
}

template <typename T>
class DBTest : public BaseDBTest<T> {
   public:
    void SetUp() override {
        BaseDBTest<T>::SetUp();

        this->db.execute(getTemplateDB<T>(), {{}});

        EXPECT_EQ(this->db.getLastInsertedRowId(), 2);

        EXPECT_EQ(this->db
                      .executeAndGetFirstInt(
                          "SELECT id from user where name = @name order by id;",
                          {{"@name", std::string("user1")}})
                      .value_or(-1),
                  1);
    }
};

// Templated suit test
TYPED_TEST_SUITE(DBTest, TestDBTypes);

TYPED_TEST(DBTest, ShouldExecuteQueryReader) {
    auto reader = this->db.executeReader(
        "SELECT id, name, email from user order by id;", {});

    EXPECT_TRUE(reader);

    int count = 0;
    std::shared_ptr<IDBRowReader> row;
    while (reader->readRow(row)) {
        EXPECT_EQ(row->readInt32(0), count + 1);
        std::string name = row->readString(1);
        std::string email = row->readString(2);

        EXPECT_EQ(email, name + "@email.com");
        count++;
    }

    EXPECT_EQ(count, 2);
}

TYPED_TEST(DBTest, TransactionsCommit) {
    this->db.begin();
    this->db.execute("delete from user where id=1;", {{}});
    this->db.commit();
    EXPECT_EQ(this->db.getChangesCount(), 1);
    EXPECT_EQ(this->db.executeAndGetFirstInt("select count(*) from user;", {{}})
                  .value_or(-1),
              1);
}

TYPED_TEST(DBTest, TransactionsRollback) {
    this->db.begin();
    this->db.execute("delete from user where 1=1;", {{}});
    this->db.rollback();
    EXPECT_EQ(this->db.executeAndGetFirstInt("select count(*) from user;", {{}})
                  .value_or(-1),
              2);
    // EXPECT_EQ(db.getChangesCount(), 0); // won't work, "by the most recently
    // completed statement"
}