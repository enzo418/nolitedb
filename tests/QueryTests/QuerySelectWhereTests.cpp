#include <gtest/gtest.h>

#include "QueryBaseNumbers.hpp"
#include "nldb/Collection.hpp"
#include "nldb/Common.hpp"
#include "nldb/Property/PropertyExpression.hpp"

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

TYPED_TEST(QueryWhereTests, ShouldSelectCorrectConditionAnd) {
    Collection numbers = this->q.collection("numbers");
    auto [name, integer_rep] = numbers.get("name", "integer_rep");

    auto expr = (name == "pi" || name == "e" || name == "imaginary number") &&
                integer_rep == 1;

    // it should be translated to the SQL equivalent:

    // select number that has one of those name and the integer_rep is 1
    // (n == "pi" || n ==  "e" || n ==  "imaginary number") && int_rep == 1

    // and not

    // n == "pi" || n ==  "e" || n ==  "imaginary number" && int_rep == 1

    // which are logically different because it will select all the numbers

    json res = this->q.from("numbers").select(name).where(expr).execute();

    ASSERT_EQ(res.size(), 1);

    ASSERT_EQ(countMembers(res[0]), 1);

    ASSERT_EQ("imaginary number", res[0]["name"]);

    /**
     * Note: it results in the following query
        (
            (
                (
                    (name_3508347910859261312.value = 'pi')
                    OR
                    (name_3508347910859261312.value = 'pi')
                )
                OR
                (name_3508347910859261312.value = 'e')
            )
            OR (name_3508347910859261312.value = 'imaginary number')
        ) AND (integerrep_3508347910859261184.value = 1)
     */
}

// same test as above but check the "wrong" condition result
TYPED_TEST(QueryWhereTests, ShouldSelectCorrectConditionAndWithoutParentheses) {
    Collection numbers = this->q.collection("numbers");
    auto [name, integer_rep] = numbers.get("name", "integer_rep");

    // prepare the condition
    auto expr = name == (std::string)this->data_numbers[0]["name"];

    for (auto& data_number : this->data_numbers) {
        expr = expr || name == (std::string)data_number["name"];
    }

    expr = name == "pi" || name == "e" ||
           name == "imaginary number" && integer_rep == 1;

    // result: selects all the numbers

    json res = this->q.from("numbers").select(name).where(expr).execute();

    ASSERT_EQ(res.size(), 3);
}

// same test as above but looped
TYPED_TEST(QueryWhereTests, ShouldSelectCorrectConditionAndLooped) {
    Collection numbers = this->q.collection("numbers");
    auto [name, integer_rep] = numbers.get("name", "integer_rep");

    // prepare the condition
    auto expr = name == (std::string)this->data_numbers[0]["name"];

    for (auto& data_number : this->data_numbers) {
        expr = expr || name == (std::string)data_number["name"];
    }

    // result: it should do the same as the ShouldSelectCorrectConditionAnd test

    json res = this->q.from("numbers")
                   .select(name)
                   .where(expr && integer_rep == 1)
                   .execute();

    ASSERT_EQ(res.size(), 1);

    ASSERT_EQ(countMembers(res[0]), 1);

    ASSERT_EQ("imaginary number", res[0]["name"]);
}

TYPED_TEST(QueryWhereTests, ShouldSelectCorrectConditionOr) {
    Collection numbers = this->q.collection("numbers");
    auto [name, integer_rep, double_rep] =
        numbers.get("name", "integer_rep", "double_rep");

    auto expr = double_rep > 0 && (name == "pi" || integer_rep >= 1);
    // dr > 0 excludes "imaginary number"
    // name == "pi" includes "pi"
    // integer_rep includes both "e" and "imaginary number" but since we said
    // that dr > 0 the result should be without the last one.

    // should NOT be translated to
    // dr > 0 && n == "pi" || ir >= 1
    // which would get all where ir >= 1, which would include "imaginary number"

    json res = this->q.from("numbers").select(name).where(expr).execute();

    ASSERT_EQ(res.size(), 2);

    ASSERT_NE("imaginary number", res[0]["name"]);
    ASSERT_NE("imaginary number", res[1]["name"]);
}

// This test show that given A and B
//
//          c++                             logic
// A:   p && (q || t) && k      ==      (p ∧ ((q ∨ t) ∧ k))
// B:   p && q || t && k        ==      ((p ∧ q) ∨ (t ∧ k))
//
// they are not equal, ( p && (q || t) && k ) != ( p && q || t && k )
TYPED_TEST(QueryWhereTests, ShouldSelectCorrectConditionAndLeftRight) {
    Collection numbers = this->q.collection("numbers");
    auto [name, integer_rep, double_rep] =
        numbers.get("name", "integer_rep", "double_rep");

    // 1. n > 2 && (w == 3 || z == "test").group() && j != 2
    // 2. n > 2 && w == 3 || z == "test" && j != 2

    // truth table
    // -----------------
    // k	p	q	t   =

    // 1. p && (q || t) && k == (p ∧ ((q ∨ t) ∧ k))
    // F	T	T	F	F

    // 2. p && q || t && k == ((p ∧ q) ∨ (t ∧ k))
    // F	T	T	F	T

    // p to T => integer_rep > 0
    // q to T => name == "pi"
    // t to F => integer_rep > 4
    // k to F => double_rep <= 2

    json res1 = this->q.from("numbers")
                    .select(name)
                    .where(integer_rep > 0 &&
                           (name == "pi" || integer_rep > 4) && double_rep <= 2)
                    .execute();

    json res2 = this->q.from("numbers")
                    .select(name)
                    .where((integer_rep > 0 && name == "pi") ||
                           (integer_rep > 4 && double_rep <= 2))
                    .execute();

    json res2_eq = this->q.from("numbers")
                       .select(name)
                       .where(integer_rep > 0 && name == "pi" ||
                              integer_rep > 4 && double_rep <= 2)
                       .execute();

    ASSERT_EQ(res1.size(), 0);
    ASSERT_EQ(res2.size(), 1);
    ASSERT_EQ(res2_eq.size(), 1);

    ASSERT_EQ("pi", (std::string)res2[0]["name"]);
    ASSERT_EQ("pi", (std::string)res2_eq[0]["name"]);
}