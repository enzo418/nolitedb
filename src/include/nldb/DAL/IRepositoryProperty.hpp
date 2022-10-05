#pragma once

#include <optional>
#include <string>
#include <vector>

#include "nldb/Property/Property.hpp"

namespace nldb {
    class IRepositoryProperty {
       public:
        virtual int add(const std::string& name, int collectionID,
                        PropertyType type) = 0;
        virtual std::optional<Property> find(int collectionID,
                                             const std::string& propName) = 0;
        virtual bool exists(int collectionID, const std::string& propName) = 0;
        virtual std::vector<Property> find(int collectionId) = 0;
    };
}  // namespace nldb