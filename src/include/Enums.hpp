#pragma once

#include <stdexcept>
enum Operator { EQ, NEQ, GT, GTE, LT, LTE, LIKE, NLIKE, AND, OR, NOT };

enum PropertyType { NUMERIC, STRING, RESERVED };

inline const char* OperatorToString(Operator v) {
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