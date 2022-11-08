#pragma once

#include "../DBBaseTest.hpp"
#include "backends/sqlite3/DB/DB.hpp"
#include "gtest/gtest.h"
#include "nldb/Common.hpp"
#include "nldb/Query/Query.hpp"
#include "nldb/SQL3Implementation.hpp"

using namespace nldb;

template <typename T>
class QueryBaseTest : public BaseDBTest<T> {
   public:
    QueryBaseTest() : q(nullptr) {}  // initialized in SetUp

   public:
    virtual void SetUp() override {
        BaseDBTest<T>::SetUp();

        q = Query(&this->db);
    }

    Query<T> q;
};

inline bool contains(const std::vector<std::string>& l, std::string s) {
    return std::find(l.begin(), l.end(), s) != l.end();
}

// compares two json objects without comparing the internal id
inline bool equalObjectsIgnoreID(json& a, json& b,
                                 const std::vector<std::string>& ignore = {}) {
    if (!a.is_object()) {
        ADD_FAILURE() << "a is not an object: \n a: " << a << "\n b: " << b;
    }

    if (!b.is_object()) {
        ADD_FAILURE() << "b is not an object: \n b: " << b << " \n a: " << a;
    }

    for (auto& [k1, v1] : a.items()) {
        if (k1 != common::internal_id_string && !contains(ignore, k1) &&
            !a[k1].is_object()) {
            if (a[k1] != b[k1]) {
                ADD_FAILURE() << a[k1] << " is not equal to " << b[k1];
                return false;
            }
        }

        if (a[k1].is_object() && !equalObjectsIgnoreID(a[k1], b[k1], ignore))
            return false;
    }

    return true;
}