#pragma once

#include <optional>

#include "nldb/Property/Property.hpp"
#include "nldb/typedef.hpp"

namespace nldb {

    class IValuesDAO {
       public:
        /**
         * @brief Adds a new value to an object
         *
         * @param propID
         * @param objID
         * @param type any type but OBJECT is allowed.
         * @param value
         */
        virtual void addStringLike(snowflake propID, snowflake objID,
                                   PropertyType type, std::string value) = 0;

        /**
         * @brief Adds a new object.
         * If objID is not present then it's a document.
         *
         * @param propID
         * @param objID
         * @param id use this id as the object id
         */
        virtual snowflake addObject(snowflake propID,
                                    std::optional<snowflake> objID) = 0;

        /**
         * @brief Adds a new object with a defined id.
         *
         * @param id defined id
         * @param propID
         * @param objID parent object id
         * @return snowflake
         */
        virtual snowflake addObjectWithID(snowflake id, snowflake propID,
                                          std::optional<snowflake> objID) = 0;

        /**
         * @brief Updates a document property value.
         *
         * @param propID
         * @param objID
         * @param type any type but OBJECT is allowed.
         * @param value
         */
        virtual void updateStringLike(snowflake propID, snowflake objID,
                                      PropertyType type, std::string value) = 0;

        /**
         * @brief Check if a document property has value.
         *
         * @param propID
         * @param objID
         * @param type
         * @return true
         * @return false
         */
        virtual bool exists(snowflake propID, snowflake objID,
                            PropertyType type) = 0;

        virtual bool existsObject(snowflake objID) = 0;

        /**
         * @brief finds a value object given its prop id and the parent id
         *
         * @param propID
         * @param objID
         * @return optional<snowflake_size>
         */
        virtual std::optional<snowflake> findObjectId(snowflake propID,
                                                      snowflake objID) = 0;

        /**
         * @brief Removes an object and all the values associated with it.
         *
         * @param objID
         */
        virtual void removeObject(snowflake objID) = 0;

        virtual ~IValuesDAO() = default;
    };
}  // namespace nldb