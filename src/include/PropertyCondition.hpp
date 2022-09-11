#pragma once
#include <string>
#include <variant>

#include "Enums.hpp"

class PropertyRep;

class PropertyCondition {
   public:
    PropertyCondition(PropertyRep* lf, Condition ct, PropertyRep* rt);
    PropertyCondition(PropertyRep* lf, Condition ct, const std::string& rt);
    PropertyCondition(PropertyRep* lf, Condition ct, int rt);

   protected:
    PropertyRep* left;
    Condition condition;
    std::variant<PropertyRep*, std::string, int> right;
};