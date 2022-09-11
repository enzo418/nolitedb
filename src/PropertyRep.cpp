#include "PropertyRep.hpp"

PropertyRep::PropertyRep(const std::string& pName) : name(pName) {};

std::string_view PropertyRep::getName() { return name; }

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