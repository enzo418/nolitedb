#include "nldb/DAL/Buffer/BufferedRepositoryCollection.hpp"

#include <optional>

#include "nldb/Collection.hpp"
#include "nldb/DAL/IRepositoryCollection.hpp"
#include "nldb/LOG/log.hpp"
#include "nldb/Property/Property.hpp"
#include "nldb/Utils/Thread.hpp"
#include "nldb/typedef.hpp"

namespace nldb {
    BufferedRepositoryCollection::BufferedRepositoryCollection(
        std::unique_ptr<IRepositoryCollection> pRepo,
        const std::shared_ptr<BufferData>& pBufferData)
        : repo(std::move(pRepo)), bufferData(pBufferData) {}

    snowflake BufferedRepositoryCollection::add(const std::string& name,
                                                snowflake ownerID) {
        snowflake id = SnowflakeGenerator::generate(getThreadID());

        bufferData->add(BufferValueCollection {
            .id = id, .name = name, .owner_id = ownerID});

        return id;
    }

    std::optional<Collection> BufferedRepositoryCollection::find(
        const std::string& name) {
        if (bufferData) {
            NLDB_PERF_FAIL("-- BUFFERED COLL -- cache MISBEHAVING");
            bufferData->pushPendingData();
        }

        return repo->find(name);
    }

    bool BufferedRepositoryCollection::exists(const std::string& name) {
        return find(name).has_value();
    }

    std::optional<Collection> BufferedRepositoryCollection::find(snowflake id) {
        if (bufferData) {
            NLDB_PERF_FAIL("-- BUFFERED COLL -- cache MISBEHAVING");
            bufferData->pushPendingData();
        }

        return repo->find(id);
    }

    bool BufferedRepositoryCollection::exists(snowflake id) {
        return this->find(id).has_value();
    }

    std::optional<Collection> BufferedRepositoryCollection::findByOwner(
        snowflake ownerID) {
        if (bufferData) {
            NLDB_PERF_FAIL("-- BUFFERED COLL -- cache MISBEHAVING");
            bufferData->pushPendingData();
        }

        return repo->findByOwner(ownerID);
    }

    std::optional<snowflake> BufferedRepositoryCollection::getOwnerId(
        snowflake collID) {
        if (bufferData) {
            NLDB_PERF_FAIL("-- BUFFERED COLL -- cache MISBEHAVING");
            bufferData->pushPendingData();
        }

        return repo->getOwnerId(collID);
    }

    std::vector<Collection> BufferedRepositoryCollection::getAll(
        bool onlyRootCollection) {
        return repo->getAll(onlyRootCollection);
    }
}  // namespace nldb
