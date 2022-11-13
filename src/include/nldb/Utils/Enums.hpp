#pragma once

#include "Enums.hpp"
#include "nldb/Property/PropertyExpression.hpp"

namespace nldb::utils {
    inline const char* OperatorToString(PropertyExpressionOperator v) {
        switch (v) {
            case EQ:
                return "=";
            case NEQ:
                return "<>";
            case GT:
                return ">";
            case GTE:
                return ">=";
            case LT:
                return "<";
            case LTE:
                return "<=";
            case LIKE:
                return "LIKE";
            case NLIKE:
                return "NOT LIKE";
            case AND:
                return "AND";
            case OR:
                return "OR";
            case NOT:
                return "NOT";
            default:
                throw std::runtime_error("Uknown operator");
        }
    }
}  // namespace nldb::utils