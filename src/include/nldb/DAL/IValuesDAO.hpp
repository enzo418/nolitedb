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
         * @brief adds a new object to a property.
         * This means that the object is not a value of another object, instead
         * it's value of a collection (a document).
         *
         * @param propID
         * @param objID
         */
        virtual snowflake addObject(snowflake propID) = 0;

        /**
         * @brief adds a new object to an object
         *
         * @param propID
         * @param docID
         * @param objID
         */
        virtual snowflake addObject(snowflake propID, snowflake objID) = 0;

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
    };
}  // namespace nldb