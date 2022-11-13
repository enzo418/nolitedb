#include "nldb/DAL/Buffer/BufferedRepositoryProperty.hpp"

#include "nldb/LOG/log.hpp"
#include "nldb/Property/Property.hpp"
#include "nldb/SnowflakeGenerator.hpp"
#include "nldb/typedef.hpp"

namespace nldb {
    BufferedRepositoryProperty::BufferedRepositoryProperty(
        std::unique_ptr<IRepositoryProperty> pRepo,
        const std::shared_ptr<BufferData>& bufferData)
        : repo(std::move(pRepo)), bufferData(bufferData) {}

    snowflake BufferedRepositoryProperty::add(const std::string& name) {
        snowflake id = SnowflakeGenerator::generate(0);

        bufferData->add(BufferValueRootProperty {.id = id, .name = name});

        return id;
    }

    snowflake BufferedRepositoryProperty::add(const std::string& name,
                                              snowflake collectionID,
                                              PropertyType type) {
        snowflake id = SnowflakeGenerator::generate(0);

        bufferData->add(BufferValueProperty {
            .id = id, .name = name, .coll_id = collectionID, .type = type});

        return id;
    }

    std::optional<Property> BufferedRepositoryProperty::find(snowflake propID) {
        if (bufferData) {
            NLDB_PERF_FAIL("-- BUFFERED PROP -- cache MISBEHAVING");
            // bufferData->pushPendingData();
        }

        return this->repo->find(propID);
    }

    std::optional<Property> BufferedRepositoryProperty::find(
        snowflake collectionID, const std::string& propName) {
        if (bufferData) {
            NLDB_PERF_FAIL("-- BUFFERED PROP -- cache MISBEHAVING");
            // bufferData->pushPendingData();
        }

        return this->repo->find(collectionID, propName);
    }

    bool BufferedRepositoryProperty::exists(snowflake collectionID,
                                            const std::string& propName) {
        if (bufferData) {
            NLDB_PERF_FAIL("-- BUFFERED PROP -- cache MISBEHAVING");
            // bufferData->pushPendingData();
        }

        return this->find(collectionID, propName).has_value();
    }

    std::vector<Property> BufferedRepositoryProperty::findAll(
        snowflake collectionId) {
        if (bufferData) {
            NLDB_PERF_FAIL("-- BUFFERED PROP -- cache MISBEHAVING");
            bufferData->pushPendingData();
        }

        return repo->findAll(collectionId);
    }
}  // namespace nldb