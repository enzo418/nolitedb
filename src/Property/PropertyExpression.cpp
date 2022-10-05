#include "nldb/Property/PropertyExpression.hpp"

namespace nldb {
    PropertyExpression::PropertyExpression() {}

    PropertyExpression PropertyExpression::operator&&(
        PropertyExpression right) {
        return PropertyExpression(PropertyExpressionOperator::AND, *this,
                                  right);
    }

    PropertyExpression PropertyExpression::operator||(
        PropertyExpression right) {
        return PropertyExpression(PropertyExpressionOperator::OR, *this, right);
    }

    PropertyExpression PropertyExpression::operator~() {
        return PropertyExpression(PropertyExpressionOperator::NOT, *this,
                                  LogicConstValue {-1});
    }
}  // namespace nldb