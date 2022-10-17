#pragma once

#include <optional>
#include <string>
#include <vector>

#include "nldb/Common.hpp"
#include "nldb/DAL/BufferData.hpp"
#include "nldb/DAL/IRepositoryProperty.hpp"
#include "nldb/DB/IDB.hpp"
#include "nldb/Property/Property.hpp"
#include "nldb/typedef.hpp"

namespace nldb {
    class BufferedRepositoryProperty : public IRepositoryProperty {
       public:
        BufferedRepositoryProperty(
            IDB* connection, std::unique_ptr<IRepositoryProperty> repo,
            const std::shared_ptr<BufferData>& bufferData);

       public:
        snowflake add(const std::string& name) override;
        snowflake add(const std::string& name, snowflake collectionID,
                      PropertyType type) override;
        std::optional<Property> find(snowflake collectionID,
                                     const std::string& propName) override;
        bool exists(snowflake collectionID,
                    const std::string& propName) override;
        std::vector<Property> find(snowflake collectionId) override;

       private:
        IDB* conn;
        std::unique_ptr<IRepositoryProperty> repo;
        std::shared_ptr<BufferData> bufferData;
    };
}  // namespace nldb