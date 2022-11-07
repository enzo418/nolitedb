#include "../DBBaseTest.hpp"
#include "backends/sqlite3/DB/DB.hpp"
#include "nldb/Query/Query.hpp"
#include "nldb/SQL3Implementation.hpp"
#include "gtest/gtest.h"

using namespace nldb;

template <typename T> class QueryBaseTest : public BaseDBTest<T> {
public:
  QueryBaseTest() : q(nullptr) {} // initialized in SetUp

public:
  virtual void SetUp() override {
    BaseDBTest<T>::SetUp();

    q = Query(&this->db);
  }

  Query<T> q;
};