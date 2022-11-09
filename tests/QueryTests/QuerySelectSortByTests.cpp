#include <gtest/gtest.h>

#include "QueryBaseNumbers.hpp"
#include "nldb/Collection.hpp"
#include "nldb/Common.hpp"

template <typename T>
class QuerySortByTest : public QueryBaseTest<T> {
   public:
    void SetUp() override {
        QueryBaseTest<T>::SetUp();

        data_values = {{{"v", 1}, {"c", {{"c_v", 4}}}},
                       {{"v", 4}, {"c", {{"c_v", 1}, {"c_v2", 1}}}},
                       {{"v", 2}, {"c", {{"c_v", 1}, {"c_v2", -1}}}},
                       {{"v", 3}, {"c", {{"c_v", 2}}}}};

        this->q.from("values").insert(data_values);
    }

    json data_values;
};

TYPED_TEST_SUITE(QuerySortByTest, TestDBTypes);

TYPED_TEST(QuerySortByTest, ShouldSelectSorted) {
    Collection values = this->q.collection("values");
    json result =
        this->q.from("values").select().sortBy(values["v"].asc()).execute();

    ASSERT_EQ(result.size(), this->data_values.size());

    for (int i = 0; i < this->data_values.size(); i++) {
        ASSERT_EQ(result[i]["v"], i + 1);
    }
}

TYPED_TEST(QuerySortByTest, ShouldSelectSortedByMultiple) {
    Collection values = this->q.collection("values");
    json result = this->q.from("values")
                      .select(values["c"])
                      .sortBy(values["c.c_v"].asc(), values["c.c_v2"].desc())
                      .execute();

    ASSERT_EQ(result.size(), this->data_values.size());

    for (int i = 0; i < this->data_values.size() - 1; i++) {
        json& res1 = result[i]["c"];
        json& res2 = result[i + 1]["c"];

        // ASC Lower to greater
        ASSERT_LE(res1["c_v"], res2["c_v"]);
        if (res1["c_v"] == res2["c_v"]) {
            // If c_v == c_v then the order is defined by c_v2, bigger = first
            ASSERT_GT(res1["c_v2"], res2["c_v2"]);
        }
    }
}