#pragma once
#include <string>
#include <variant>

#include "CommonTypes.hpp"
#include "Enums.hpp"

class PropertyRep;

class PropertyCondition {
   public:
    PropertyCondition(PropertyRep* lf, Condition ct, PropertyRep* rt);
    PropertyCondition(PropertyRep* lf, Condition ct, const std::string& rt);
    PropertyCondition(PropertyRep* lf, Condition ct,
                      double rt);  // MAYBE TEMPLATE INT, DOUBLE AS T?

   public:
    std::tuple<PropertyRep*, Condition, RightValue> get() const;
    PropertyRep* getLeftExp();
    Condition getCondition();
    RightValue getRightExp();

   protected:
    PropertyRep* left;
    Condition condition;
    RightValue right;  // MAYBE TEMPLATE INT, DOUBLE AS T?
};