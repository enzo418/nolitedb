#pragma once

#include <cstdint>

#include "nldb/DAL/IValuesDAO.hpp"
#include "nldb/DB/IDB.hpp"
#include "nldb/SnowflakeGenerator.hpp"
#include "nldb/Utils/ValueBuffer.hpp"

namespace nldb {

    struct BufferValueStringLike {
        int propID;
        snowflake objID;
        PropertyType type;
        std::string value {""};
    };

    struct BufferValueDependentObject {
        snowflake id;
        int prop_id;
        snowflake obj_id;
    };

    struct BufferValueIndependentObject {
        snowflake id;
        int prop_id;
    };

    class ValuesDAO : public IValuesDAO {
       public:
        ValuesDAO(IDB* connection);

       public:
        void addStringLike(int propID, snowflake objID, PropertyType type,
                           std::string value) override;

        snowflake addObject(int propID) override;

        snowflake deferAddObject(int propID) override;

        snowflake addObject(int propID, snowflake objID) override;

        snowflake deferAddObject(int propID, snowflake objID) override;

        void updateStringLike(int propID, snowflake objID, PropertyType type,
                              std::string value) override;

        bool exists(int propID, snowflake objID, PropertyType type) override;

        bool existsObject(snowflake objID) override;

        std::optional<snowflake> findObjectId(int propID,
                                              snowflake objID) override;

        void removeObject(snowflake objID) override;

        void deferAddStringLike(int propID, snowflake objID, PropertyType type,
                                std::string value) override;

        void pushPendingData() override;

        ~ValuesDAO();

       private:
        IDB* conn;
        Buffer<BufferValueStringLike> bufferStringLike;
        Buffer<BufferValueDependentObject> bufferDependentObject;
        Buffer<BufferValueIndependentObject> bufferIndependentObject;
        NullLock lock;
    };
}  // namespace nldb