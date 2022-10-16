#pragma once

#include <string>
#include <unordered_map>
#include <variant>

#include "nldb/typedef.hpp"

typedef std::variant<std::string_view, std::string, int, snowflake_size, double>
    ParamsBindValue;

typedef std::unordered_map<std::string, ParamsBindValue>
    Paramsbind;  // use std::string_view