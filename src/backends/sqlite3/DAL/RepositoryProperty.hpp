#pragma once

#include <optional>
#include <string>
#include <vector>

#include "nldb/DAL/IRepositoryProperty.hpp"
#include "nldb/DB/IDB.hpp"
#include "nldb/Property/Property.hpp"

namespace nldb {
    class RepositoryProperty : public IRepositoryProperty {
       public:
        RepositoryProperty(IDB* connection);

       public:
        int add(const std::string& name, int collectionID,
                PropertyType type) override;
        std::optional<Property> find(int collectionID,
                                     const std::string& propName) override;
        bool exists(int collectionID, const std::string& propName) override;
        std::vector<Property> find(int collectionId) override;

       private:
        IDB* conn;
    };
}  // namespace nldb