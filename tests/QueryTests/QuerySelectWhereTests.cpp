#include <gtest/gtest.h>

#include "QueryBaseNumbers.hpp"
#include "nldb/Collection.hpp"
#include "nldb/Common.hpp"

template <typename T>
class QueryWhereTests : public QueryNumbersTest<T> {};

TYPED_TEST_SUITE(QueryWhereTests, TestDBTypes);

TYPED_TEST(QueryWhereTests, ShouldSelectWhereEqual) {
    json expected = this->data_numbers[0];

    Collection numbers = this->q.collection("numbers");
    json res = this->q.from("numbers")
                   .select()
                   .where(numbers["name"] == (std::string)expected["name"])
                   .execute();

    ASSERT_EQ(res.size(), 1);
    ASSERT_EQ(expected["name"], res[0]["name"]);
    ASSERT_NEAR(expected["double_rep"], res[0]["double_rep"], 1e-2);
    ASSERT_EQ(expected["integer_rep"], res[0]["integer_rep"]);
}

TYPED_TEST(QueryWhereTests, ShouldSelectWhereNEqual) {
    Collection numbers = this->q.collection("numbers");
    json res =
        this->q.from("numbers")
            .select()
            .where(numbers["integer_rep"] != 3 && numbers["integer_rep"] != 1)
            .execute();

    ASSERT_EQ(res.size(), 1);
    ASSERT_EQ("e", res[0]["name"]);
}

TYPED_TEST(QueryWhereTests, ShouldSelectWhereGreater) {
    Collection numbers = this->q.collection("numbers");

    json res = this->q.from("numbers")
                   .select()
                   .where(numbers["extra"]["known_since"] >= 23)
                   .execute();

    ASSERT_EQ(res.size(), 1);
    ASSERT_EQ("imaginary number", res[0]["name"]);
}

TYPED_TEST(QueryWhereTests, ShouldSelectWhereLess) {
    Collection numbers = this->q.collection("numbers");

    json res = this->q.from("numbers")
                   .select()
                   .where(numbers["integer_rep"] < 3)
                   .execute();

    ASSERT_EQ(res.size(), 2);
    ASSERT_NE("pi", res[0]["name"]);
    ASSERT_NE("pi", res[1]["name"]);
}

TYPED_TEST(QueryWhereTests, ShouldSelectWhereLike) {
    Collection numbers = this->q.collection("numbers");

    json res = this->q.from("numbers")
                   .select()
                   .where(numbers["name"] % "%e%")
                   .sortBy(numbers["extra"]["known_since"].asc())
                   .execute();

    ASSERT_EQ(res.size(), 2);
    ASSERT_LT(res[0]["extra"]["known_since"], res[1]["extra"]["known_since"]);
    ASSERT_EQ("e", res[0]["name"]);
    ASSERT_EQ("imaginary number", res[1]["name"]);
}

TYPED_TEST(QueryWhereTests, ShouldSelectWhereNotLike) {
    Collection numbers = this->q.collection("numbers");

    auto [name, extra] = numbers.get("name", "extra{definition}"_obj);

    json res = this->q.from("numbers")
                   .select(name, extra)
                   .where(name ^ "%e%" || extra["definition"] == "hello there")
                   .sortBy(numbers["extra"]["known_since"].desc())
                   .execute();

    ASSERT_EQ(res.size(), 1);
    ASSERT_EQ("pi", res[0]["name"]);

    ASSERT_TRUE(res[0]["extra"].contains("definition"));
    ASSERT_EQ(countMembers(res[0]["extra"]), 1);
}

TYPED_TEST(QueryWhereTests,
           ShouldSelectWhereMultipleConditionsWithoutSelectingThem) {
    Collection numbers = this->q.collection("numbers");
    auto [id, name, extra] = numbers.get(common::internal_id_string, "name",
                                         "extra{known_since}"_obj);

    json res = this->q.from("numbers")
                   .select(name)
                   .where(name != "pi" &&
                          extra["known_since"] != numbers["integer_rep"])
                   .where(id >= 0)  // Equivalent to && id >= 0
                   .execute();

    ASSERT_EQ(res.size(), 2);

    ASSERT_EQ(countMembers(res[0]), 1);
    ASSERT_EQ(countMembers(res[1]), 1);

    ASSERT_NE("pi", res[0]["name"]);
    ASSERT_NE("pi", res[1]["name"]);
}