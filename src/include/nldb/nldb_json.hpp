#pragma once

#include "nldb/Property/Property.hpp"
#include "nlohmann/json.hpp"

namespace nldb {
    typedef nlohmann::json json;

    PropertyType JsonTypeToPropertyType(int type);

    std::string ValueToString(json& val);
}  // namespace nldb