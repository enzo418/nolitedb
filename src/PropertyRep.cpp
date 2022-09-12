#include "PropertyRep.hpp"

SqlStatement<std::string> getStatement(PropertyRep* lf, Operator op,
                                       PropertyRep& rt) {
    return SqlStatement<std::string>(lf->getStatement() + " " +
                                     OperatorToString(op) + " " +
                                     rt.getStatement());
}

SqlStatement<std::string> getStatement(PropertyRep* lf, Operator op,
                                       const std::string& rt) {
    return SqlStatement<std::string>(lf->getStatement() + " " +
                                     OperatorToString(op) + " " + rt);
}

PropertyRep::PropertyRep(const std::string& pName) : name(pName) {};

std::string_view PropertyRep::getName() { return name; }

std::string PropertyRep::getStatement() const {
    switch (this->type) {
        case PropertyType::NUMERIC:
            return this->name + "_vi";
        case PropertyType::STRING:
            return this->name + "_vs";
        default:
            return this->name;
    }
}

// PropertyCondition PropertyRep::operator<(PropertyRep& rt) {
//     return PropertyCondition(this, Condition::LT, &rt);
// }

// PropertyCondition PropertyRep::operator<=(PropertyRep& rt) {
//     return PropertyCondition(this, Condition::LTE, &rt);
// }

// PropertyCondition PropertyRep::operator>(PropertyRep& rt) {
//     return PropertyCondition(this, Condition::GT, &rt);
// }

// PropertyCondition PropertyRep::operator>=(PropertyRep& rt) {
//     return PropertyCondition(this, Condition::GTE, &rt);
// }

// PropertyCondition PropertyRep::operator==(PropertyRep& rt) {
//     return PropertyCondition(this, Condition::EQ, &rt);
// }

// PropertyCondition PropertyRep::operator!=(PropertyRep& rt) {
//     return PropertyCondition(this, Condition::NEQ, &rt);
// }

// PropertyCondition PropertyRep::operator%(PropertyRep& rt) {
//     return PropertyCondition(this, Condition::LIKE, &rt);
// }

// PropertyCondition PropertyRep::operator^(PropertyRep& rt) {
//     return PropertyCondition(this, Condition::NLIKE, &rt);
// }