#pragma once
#include <string>

namespace nldb::common {
    constexpr const char* internal_id_string = "__id__";

    std::string inline getSubCollectionName(const std::string& collName,
                                            const std::string& propName) {
        return "_" + collName + "_" + propName;
    }
}  // namespace nldb::common
