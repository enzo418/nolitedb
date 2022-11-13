#pragma once

#include <gtest/gtest.h>

#include "QueryBase.hpp"

template <typename T>
class QueryNumbersTest : public QueryBaseTest<T> {
   public:
    void SetUp() override {
        QueryBaseTest<T>::SetUp();

        this->data_numbers = {
            {{"name", "pi"},
             {"double_rep", M_PI},
             {"integer_rep", 3},
             {"extra",
              {{"definition", "circumference/diameter"},
               {"known_since", -100}}}},

            {{"name", "e"},
             {"double_rep", M_E},
             {"integer_rep", 2},
             {"extra",
              {{"definition", "lim n -> inf (1 + 1/n)^n"},
               {"known_since", -50}}}},

            {{"name", "imaginary number"},
             {"double_rep", -1.0},
             {"integer_rep", 1},
             {"extra", {{"definition", "i^2 = -1"}, {"known_since", 23}}}}};

        data_numbers_usage = {{{"name", "pi"}, {"used", 100}},
                              {{"name", "log2(e)"}, {"used", 50}}};

        this->q.from("numbers").insert(data_numbers);
        this->q.from("numbers_usage").insert(data_numbers_usage);
    }

    json data_numbers;
    json data_numbers_usage;
};

TYPED_TEST_SUITE(QueryNumbersTest, TestDBTypes);