#pragma once

#include <unordered_map>

#include "nldb/Property/Property.hpp"

namespace nldb::definitions {
    /**
     * @brief Why this file is in SQLite 3 implementation?
     * Because there might be implementations that doesn't need SQL tables.
     */
    namespace tables {
        std::unordered_map<PropertyType, std::string>& getPropertyTypesTable();
    }

    std::string inline getSubCollectionName(const std::string& collName,
                                            const std::string& propName) {
        return "_" + collName + "_" + propName;
    }
}  // namespace nldb::definitions