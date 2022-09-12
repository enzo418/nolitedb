#pragma once

#include <string>

#include "Enums.hpp"
#include "ISqlStatement.hpp"

template <typename T>
class SqlStatement : public ISqlStatement {
   public:
    SqlStatement(T val);

   public:
    std::string getStatement() const override;

   private:
    T val;
};

template <>
class SqlStatement<std::string> : public ISqlStatement {
   public:
    SqlStatement(const std::string& str) : val(str) {};

   public:
    std::string getStatement() const override { return this->val; }

   public:
    SqlStatement<std::string> operator~() {
        return std::string(OperatorToString(Operator::NOT)) + " " +
               this->getStatement();
    }

    SqlStatement<std::string> operator&&(SqlStatement<std::string> rt) {
        return this->getStatement() + " " + OperatorToString(Operator::AND) +
               " " + rt.getStatement();
    }

    SqlStatement<std::string> operator||(SqlStatement<std::string> rt) {
        return this->getStatement() + " " + OperatorToString(Operator::OR) +
               " " + rt.getStatement();
    }

   private:
    std::string val;
};

template <>
class SqlStatement<double> : public ISqlStatement {
   public:
    SqlStatement(double v) : val(v) {};

   public:
    std::string getStatement() const override {
        return std::to_string(this->val);
    }

   private:
    double val;
};

template <>
class SqlStatement<int> : public ISqlStatement {
   public:
    SqlStatement(int v) : val(v) {};

   public:
    std::string getStatement() const override {
        return std::to_string(this->val);
    }

   private:
    int val;
};