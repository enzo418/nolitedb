#pragma once
#include <string>
#include <variant>

#include "CommonTypes.hpp"
#include "Enums.hpp"
#include "ISqlStatement.hpp"
#include "SqlStatement.hpp"

class PropertyRep;

// class PropertyCondition {
//    public:
//     PropertyCondition(PropertyRep* lf, Condition ct, PropertyRep* rt);
//     PropertyCondition(PropertyRep* lf, Condition ct, const std::string& rt);
//     PropertyCondition(PropertyRep* lf, Condition ct,
//                       double rt);  // MAYBE TEMPLATE INT, DOUBLE AS T?

//     // TODO:
//     // PropertyConditionUpper operator&&(const PropertyConditionUpper&)
//     // PropertyConditionUpper operator&&(const PropertyCondition&)
//    public:
//     std::tuple<PropertyRep*, Condition, RightValue> get() const;
//     PropertyRep* getLeftExp();
//     Condition getCondition();
//     RightValue getRightExp();

//    protected:
//     PropertyRep* left;
//     Condition condition;
//     RightValue right;  // MAYBE TEMPLATE INT, DOUBLE AS T?
// };

// template <typename L, typename R>
// class SqlExpression : public ISqlStatement {
//    public:
//     SqlExpression(SqlStatement<L> lf, Operator ct, SqlStatement<R> rt);

//     std::string getStatement() const override;

//     SqlExpression operator~();
//     SqlExpression operator&&(SqlStatement<typename T>);
//     SqlExpression operator||(SqlExpression);
// };

// template <typename L, typename R>
// SqlExpression<L, R>::SqlExpression(SqlStatement<L> lf, Operator ct,
//                                    SqlStatement<R> rt) {
//     this->left = lf;
//     this->op = ct;
//     this->right = rt;
// }

// template <typename L, typename R>
// SqlExpression SqlExpression<L, R>::operator&&(SqlExpression re) {
//     return SqlExpression(this, Operator::AND, &re);
// }

// template <typename L, typename R>
// SqlExpression SqlExpression<L, R>::operator||(SqlExpression& re) {
//     return SqlExpression(this, Operator::OR, &re);
// }
// template <typename L, typename R>
// SqlExpression SqlExpression<L, R>::operator~() {
//     return SqlExpression(this, Operator::NOT, nullptr);
// }
// template <typename L, typename R>
// std::string SqlExpression<L, R>::getStatement() const {
//     if (this->op == Operator::NOT) {
//         return "NOT " + this->left->getStatement();
//     }

//     return this->left->getStatement() + OperatorToString(this->op) +
//            this->right->getStatement();
// }

// template <typename T>
// SqlExpression::SqlExpression(PropertyRep* lf, Operator ct,
//                              SqlStatement<T>&& rt) {
//     this->left = (ISqlStatement*)lf;
//     this->op = ct;
//     this->right = rt;
// }