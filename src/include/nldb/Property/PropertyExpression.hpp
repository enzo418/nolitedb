#pragma once

#include <variant>

#include "nldb/Property/Property.hpp"
#include "nldb/box.hpp"

namespace nldb {
    enum PropertyExpressionOperator {
        EQ,
        NEQ,
        GT,
        GTE,
        LT,
        LTE,
        LIKE,
        NLIKE,
        AND,
        OR,
        NOT
    };

    typedef std::variant<LogicConstValue, box<struct PropertyExpression>>
        PropertyExpressionOperand;

    struct PropertyExpression {
        PropertyExpression();

        template <typename L, typename R>
        PropertyExpression(PropertyExpressionOperator pType, L pLeft,
                           R pRight) {
            type = pType;
            left = pLeft;
            right = pRight;
        }

        PropertyExpressionOperator type;
        PropertyExpressionOperand left {0};
        PropertyExpressionOperand right {0};

        PropertyExpression operator&&(PropertyExpression right);
        PropertyExpression operator||(PropertyExpression right);
        PropertyExpression operator~();
    };

    // i guess one could have PropertyExpressionAnd, PropertyExpressionOr, ...
}  // namespace nldb