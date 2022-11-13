#include <gtest/gtest.h>

#include "QueryBase.hpp"
#include "QueryBaseCars.hpp"
#include "nldb/Collection.hpp"
#include "nldb/Common.hpp"
#include "nldb/typedef.hpp"

using namespace nldb;

template <typename T>
class QueryRemoveTestsCars : public QueryCarsTest<T> {};
TYPED_TEST_SUITE(QueryRemoveTestsCars, TestDBTypes);

TYPED_TEST(QueryRemoveTestsCars, ShouldRemoveCars) {
    json result = this->q.from("cars").select().execute();

    ASSERT_EQ(result.size(), this->data_cars.size());

    int prevSize = result.size();
    for (auto& car : result) {
        this->q.from("cars").remove(
            car[common::internal_id_string].get<std::string>());

        ASSERT_EQ(this->q.from("cars").select().execute().size(), prevSize - 1);
        prevSize--;
    }
}

TYPED_TEST(QueryRemoveTestsCars, ShouldRemoveInnerDocument) {
    Collection cars = this->q.collection("cars");
    json result = this->q.from("cars")
                      .select()
                      .page(1, 1)
                      .sortBy(cars["year"].asc())
                      .execute();

    ASSERT_EQ(result.size(), 1);

    this->q.from("cars").remove(
        result[0]["technical"][common::internal_id_string].get<std::string>());

    json afterRemove = this->q.from("cars")
                           .select()
                           .page(1, 1)
                           .sortBy(cars["year"].asc())
                           .execute();

    ASSERT_TRUE(result[0].contains("technical"));

    ASSERT_EQ(afterRemove.size(), 1);
    ASSERT_FALSE(afterRemove[0].contains("technical"));
}