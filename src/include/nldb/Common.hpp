#pragma once
#include <string>

#ifndef NLDB_INTERNAL_ID
#include "nldb_config.h"
#endif

namespace nldb::common {
    constexpr const char* internal_id_string = NLDB_INTERNAL_ID;

    std::string inline getSubCollectionName(const std::string& collName,
                                            const std::string& propName) {
        return "_" + collName + "_" + propName;
    }
}  // namespace nldb::common
