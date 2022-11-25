#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "nldb/typedef.hpp"

typedef std::variant<std::string_view, std::string, int, nldb::snowflake,
                     double>
    ParamsBindValue;

typedef std::vector<std::pair<std::string, ParamsBindValue>>
    Paramsbind;  // use std::string_view