#pragma once

#include <memory>

#include "lrucache11/LRUCache11.hpp"
#include "nldb/Collection.hpp"
#include "nldb/DAL/BufferData.hpp"
#include "nldb/DAL/IRepositoryCollection.hpp"
#include "nldb/DB/IDB.hpp"
#include "nldb/SnowflakeGenerator.hpp"
#include "nldb/Utils/Hash.hpp"

namespace nldb {
    class BufferedRepositoryCollection : public IRepositoryCollection {
       public:
        BufferedRepositoryCollection(
            std::unique_ptr<IRepositoryCollection> repo,
            const std::shared_ptr<BufferData>& bufferData);

       public:
        snowflake add(const std::string& name, snowflake ownerID) override;
        std::optional<Collection> find(const std::string& name) override;
        bool exists(const std::string& name) override;
        std::optional<Collection> find(snowflake id) override;
        bool exists(snowflake id) override;
        std::optional<Collection> findByOwner(snowflake ownerID) override;
        std::optional<snowflake> getOwnerId(snowflake collID) override;
        std::vector<Collection> getAll(bool onlyRootCollection) override;

       private:
        std::unique_ptr<IRepositoryCollection> repo;
        std::shared_ptr<BufferData> bufferData;
    };
}  // namespace nldb