#pragma once
#include <string>

#include "Enums.hpp"
#include "PropertyCondition.hpp"

class PropertyRep {
   public:
    PropertyRep(const std::string& pName);

   public:
    PropertyCondition operator<(PropertyRep& rt);  // LESS THAN

    PropertyCondition operator<=(PropertyRep& rt);  // LESS THAN EQUAL

    PropertyCondition operator>(PropertyRep& rt);  // GREATER THAN

    PropertyCondition operator>=(PropertyRep& rt);  // GREATER THAN EQUAL

    PropertyCondition operator==(PropertyRep& rt);  // EQUAL

    PropertyCondition operator!=(PropertyRep& rt);  // NOT EQUAL

    PropertyCondition operator%(PropertyRep& rt);  // LIKE

    PropertyCondition operator^(PropertyRep& rt);  // NOT LIKE

   public:
    std::string_view getName();

   protected:
    std::string name;
};