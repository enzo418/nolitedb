#include "SqlExpression.hpp"

SqlLogicExpression::SqlLogicExpression(std::string st,
                                       std::vector<PropertyRep*> pPr)
    : st(std::move(st)), actingProps(std::move(pPr)) {}

std::string SqlLogicExpression::getStatement() const { return std::move(st); }

std::vector<PropertyRep*> SqlLogicExpression::getActingProps() const {
    return std::move(actingProps);
}

SqlLogicExpression SqlLogicExpression::operator&&(const SqlLogicExpression& q) {
    actingProps.insert(actingProps.end(), q.actingProps.begin(),
                       q.actingProps.end());
    return SqlLogicExpression(st + " " + OperatorToString(AND) + " " + q.st,
                              std::move(actingProps));
}

SqlLogicExpression SqlLogicExpression::operator||(const SqlLogicExpression& q) {
    actingProps.insert(actingProps.end(), q.actingProps.begin(),
                       q.actingProps.end());
    return SqlLogicExpression(st + " " + OperatorToString(OR) + " " + q.st,
                              std::move(actingProps));
}

SqlLogicExpression SqlLogicExpression::operator~() {
    return SqlLogicExpression(OperatorToString(NOT) + std::string(" ") + st,
                              std::move(actingProps));
}