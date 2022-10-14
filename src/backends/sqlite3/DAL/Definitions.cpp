#include <unordered_map>

#include "nldb/Property/Property.hpp"

namespace nldb::definitions {
    namespace tables {
        std::unordered_map<PropertyType, std::string>& getPropertyTypesTable() {
            static std::unordered_map<PropertyType, std::string>
                propertyTypeTable = {{PropertyType::STRING, "value_string"},
                                     {PropertyType::INTEGER, "value_int"},
                                     {PropertyType::DOUBLE, "value_double"},
                                     {PropertyType::ARRAY, "value_array"},
                                     {PropertyType::OBJECT, "object"}};

            return propertyTypeTable;
        }
    }  // namespace tables
}  // namespace nldb::definitions