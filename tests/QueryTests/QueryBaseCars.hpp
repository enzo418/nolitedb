#pragma once

#include <gtest/gtest.h>

#include "QueryBase.hpp"

template <typename T>
class QueryCarsTest : public QueryBaseTest<T> {
   public:
    void SetUp() override {
        QueryBaseTest<T>::SetUp();

        // {name, founded, country}
        data_automaker = {{{"name", "ford"},
                           {"founded", "June 16, 1903"},
                           {"country", "United States"}},
                          // ---
                          {{"name", "subaru"},
                           {"founded", "July 15, 1953"},
                           {"country", "Japan"}}};

        // {maker, model, year}
        data_cars = {
            {{"maker", "ford"},
             {"model", "focus"},
             {"year", 2011},
             {"categories", {1, 9, 5}},
             {"technical",
              {{"length", 4}, {"weight", "1331 kg"}, {"0-60 mph", 4.8}}}},
            // --
            {{"maker", "ford"},
             {"model", "focus"},
             {"year", 2015},
             {"categories", {1, 3, 4}},
             {"technical", {{"width", 2.1}, {"0-60 mph", 5.8}}}},
            // --
            {{"maker", "subaru"},
             {"model", "impreza"},
             {"year", 2003},
             {"categories", {2, 7, 5}},
             {"technical", {{"0-60 mph", 3.1}}}}};

        this->q.from("automaker").insert(data_automaker);
        this->q.from("cars").insert(data_cars);
    }

    json data_automaker;
    json data_cars;
};

TYPED_TEST_SUITE(QueryCarsTest, TestDBTypes);
