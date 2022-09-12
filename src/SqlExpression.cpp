#include "SqlExpression.hpp"

#include <tuple>

// SqlExpression::SqlExpression(ISqlStatement* lf, Operator ct, ISqlStatement*
// rt)
//     : left(lf), op(ct), right(rt) {}

// SqlExpression SqlExpression::operator&&(SqlExpression& re) {
//     return SqlExpression(this, Operator::AND, &re);
// }

// SqlExpression SqlExpression::operator||(SqlExpression& re) {
//     return SqlExpression(this, Operator::OR, &re);
// }

// SqlExpression SqlExpression::operator~() {
//     return SqlExpression(this, Operator::NOT, nullptr);
// }

// std::string SqlExpression::getStatement() const {
//     if (this->op == Operator::NOT) {
//         return "NOT " + this->left->getStatement();
//     }

//     return this->left->getStatement() + OperatorToString(this->op) +
//            this->right->getStatement();
// }

// std::tuple<PropertyRep*, Condition, RightValue> SqlExpression::get()
//     const {
//     return std::make_tuple(left, condition, right);
// }

// PropertyRep* SqlExpression::getLeftExp() { return left; }

// Condition SqlExpression::getCondition() { return condition; }

// RightValue SqlExpression::getRightExp() { return right; }