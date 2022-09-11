#include "PropertyCondition.hpp"

#include <tuple>

PropertyCondition::PropertyCondition(PropertyRep* lf, Condition ct,
                                     PropertyRep* rt)
    : left(lf), condition(ct), right(rt) {}

PropertyCondition::PropertyCondition(PropertyRep* lf, Condition ct,
                                     const std::string& rt)
    : left(lf), condition(ct), right(rt) {}

PropertyCondition::PropertyCondition(PropertyRep* lf, Condition ct, double rt)
    : left(lf), condition(ct), right(rt) {}

std::tuple<PropertyRep*, Condition, RightValue> PropertyCondition::get() const {
    return std::make_tuple(left, condition, right);
}

PropertyRep* PropertyCondition::getLeftExp() { return left; }

Condition PropertyCondition::getCondition() { return condition; }

RightValue PropertyCondition::getRightExp() { return right; }