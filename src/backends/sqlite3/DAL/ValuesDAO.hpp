#pragma once

#include <cstdint>

#include "nldb/DAL/BufferData.hpp"
#include "nldb/DAL/IValuesDAO.hpp"
#include "nldb/DB/IDB.hpp"
#include "nldb/SnowflakeGenerator.hpp"

namespace nldb {

    class ValuesDAO : public IValuesDAO {
       public:
        ValuesDAO(IDB* connection);

       public:
        void addStringLike(snowflake propID, snowflake objID, PropertyType type,
                           std::string value) override;

        snowflake addObject(snowflake propID,
                            std::optional<snowflake> objID) override;

        snowflake addObjectWithID(snowflake id, snowflake propID,
                                  std::optional<snowflake> objID) override;

        void updateStringLike(snowflake propID, snowflake objID,
                              PropertyType type, std::string value) override;

        bool exists(snowflake propID, snowflake objID,
                    PropertyType type) override;

        bool existsObject(snowflake objID) override;

        std::optional<snowflake> findObjectId(snowflake propID,
                                              snowflake objID) override;

        void removeObject(snowflake objID) override;

       private:
        IDB* conn;
    };
}  // namespace nldb