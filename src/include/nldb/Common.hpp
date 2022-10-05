#pragma once
#include <string>

namespace nldb::common {
    std::string inline getSubCollectionName(const std::string& collName,
                                            const std::string& propName) {
        return "_" + collName + "_" + propName;
    }
}  // namespace nldb::common
