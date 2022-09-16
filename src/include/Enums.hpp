#pragma once

#include <stdexcept>
enum Operator { EQ, NEQ, GT, GTE, LT, LTE, LIKE, NLIKE, AND, OR, NOT };

enum PropertyType { INTEGER = 1, DOUBLE, STRING, RESERVED };

enum AGGREGATEFUNCTIONTYPE { COUNT, AVG, SUM, MAX, MIN };

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

inline const char* AggregatefunctionTypeToString(AGGREGATEFUNCTIONTYPE v) {
    switch (v) {
        case AVG:
            return "AVG";
        case COUNT:
            return "COUNT";
        case MAX:
            return "MAX";
        case MIN:
            return "MIN";
        case SUM:
            return "SUM";
        default:
            throw std::runtime_error("Uknown operator");
    }
}