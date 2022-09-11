#include "PropertyCondition.hpp"

PropertyCondition::PropertyCondition(PropertyRep* lf, Condition ct,
                                     PropertyRep* rt)
    : left(lf), condition(ct), right(rt) {}

PropertyCondition::PropertyCondition(PropertyRep* lf, Condition ct,
                                     const std::string& rt)
    : left(lf), condition(ct), right(rt) {}

PropertyCondition::PropertyCondition(PropertyRep* lf, Condition ct, int rt)
    : left(lf), condition(ct), right(rt) {}