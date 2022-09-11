#pragma once
#include <string>
#include <variant>

class PropertyRep;

typedef std::variant<PropertyRep*, std::string, double> RightValue;