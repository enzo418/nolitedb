#include "SqlExpression.hpp"

SqlLogicExpression::SqlLogicExpression(std::string st,
                                       std::set<PropertyRep*> pPr)
    : st(std::move(st)), actingProps(std::move(pPr)) {}

std::string SqlLogicExpression::getStatement() const { return std::move(st); }

std::set<PropertyRep*> SqlLogicExpression::getActingProps() const {
    return std::move(actingProps);
}

SqlLogicExpression SqlLogicExpression::operator&&(const SqlLogicExpression& q) {
    utils::concat(actingProps, q.actingProps);
    return SqlLogicExpression(st + " " + OperatorToString(AND) + " " + q.st,
                              std::move(actingProps));
}

SqlLogicExpression SqlLogicExpression::operator||(const SqlLogicExpression& q) {
    utils::concat(actingProps, q.actingProps);
    return SqlLogicExpression(st + " " + OperatorToString(OR) + " " + q.st,
                              std::move(actingProps));
}

SqlLogicExpression SqlLogicExpression::operator~() {
    return SqlLogicExpression(OperatorToString(NOT) + std::string(" ") + st,
                              std::move(actingProps));
}