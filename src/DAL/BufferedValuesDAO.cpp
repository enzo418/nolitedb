#include "nldb/DAL/Buffer/BufferedValuesDAO.hpp"

#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <unordered_map>

#include "magic_enum.hpp"
#include "nldb/DAL/IValuesDAO.hpp"
#include "nldb/LOG/log.hpp"
#include "nldb/Property/Property.hpp"
#include "nldb/Utils/ParamsBindHelpers.hpp"
#include "nldb/Utils/Thread.hpp"

#define USE_BUFFER

namespace nldb {
    BufferedValuesDAO::BufferedValuesDAO(
        std::unique_ptr<IValuesDAO>&& pRepo,
        std::shared_ptr<BufferData> pBufferData)
        : repo(std::move(pRepo)), bufferData(pBufferData) {}

    void BufferedValuesDAO::addStringLike(snowflake propID, snowflake objID,
                                          PropertyType type,
                                          std::string value) {
        bufferData->add(BufferValueStringLike {
            .propID = propID, .objID = objID, .type = type, .value = value});
    }

    snowflake BufferedValuesDAO::addObject(snowflake propID,
                                           std::optional<snowflake> objID) {
        snowflake newId = SnowflakeGenerator::generate(getThreadID());

        if (objID.has_value()) {
            bufferData->add(BufferValueDependentObject {
                .id = newId, .prop_id = propID, .obj_id = objID.value()});
        } else {
            bufferData->add(
                BufferValueIndependentObject {.id = newId, .prop_id = propID});
        }

        return newId;
    }

    snowflake BufferedValuesDAO::addObjectWithID(
        snowflake id, snowflake propID, std::optional<snowflake> objID) {
        if (objID.has_value()) {
            bufferData->add(BufferValueDependentObject {
                .id = id, .prop_id = propID, .obj_id = objID.value()});
        } else {
            bufferData->add(
                BufferValueIndependentObject {.id = id, .prop_id = propID});
        }

        return id;
    }

    void BufferedValuesDAO::updateStringLike(snowflake propID, snowflake objID,
                                             PropertyType type,
                                             std::string value) {
        repo->updateStringLike(propID, objID, type, std::move(value));
    }

    bool BufferedValuesDAO::exists(snowflake propID, snowflake objID,
                                   PropertyType type) {
        return repo->exists(propID, objID, type);
    }

    std::optional<snowflake> BufferedValuesDAO::findObjectId(snowflake propID,
                                                             snowflake objID) {
        return repo->findObjectId(propID, objID);
    }

    void BufferedValuesDAO::removeObject(snowflake objID) {
        return repo->removeObject(objID);
    }

    bool BufferedValuesDAO::existsObject(snowflake objID) {
        return repo->existsObject(objID);
    }
}  // namespace nldb