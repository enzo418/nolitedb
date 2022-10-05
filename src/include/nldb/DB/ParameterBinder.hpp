#pragma once

#include <string>
#include <unordered_map>
#include <variant>

typedef std::variant<std::string, int, double> ParamsBindValue;
typedef std::unordered_map<std::string, ParamsBindValue>
    Paramsbind;  // use std::string_view