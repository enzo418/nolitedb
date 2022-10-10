#include <unordered_map>

#include "nldb/Property/Property.hpp"

namespace nldb::definitions {
    namespace tables {
        std::unordered_map<PropertyType, std::string>& getPropertyTypesTable() {
            static std::unordered_map<PropertyType, std::string>
                propertyTypeTable = {{PropertyType::STRING, "value_string"},
                                     {PropertyType::INTEGER, "value_int"},
                                     {PropertyType::DOUBLE, "value_double"},
                                     {PropertyType::OBJECT, "value_object"}};

            return propertyTypeTable;
        }
    }  // namespace tables
}  // namespace nldb::definitions