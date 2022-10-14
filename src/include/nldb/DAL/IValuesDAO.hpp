#pragma once

#include <optional>

#include "nldb/Property/Property.hpp"

namespace nldb {
    struct ValueObjectMapped {
        int id;
        int doc_id;
        int prop_id;
        int sub_coll_id;
        int sub_doc_id;
    };

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
        virtual void addStringLike(int propID, int objID, PropertyType type,
                                   std::string value) = 0;

        /**
         * @brief adds a new object to a property.
         * This means that the object is not a value of another object, instead
         * it's value of a collection (a document).
         *
         * @param propID
         * @param objID
         */
        virtual int addObject(int propID) = 0;

        /**
         * @brief adds a new object to an object
         *
         * @param propID
         * @param docID
         * @param objID
         */
        virtual int addObject(int propID, int objID) = 0;

        /**
         * @brief Updates a document property value.
         *
         * @param propID
         * @param objID
         * @param type any type but OBJECT is allowed.
         * @param value
         */
        virtual void updateStringLike(int propID, int objID, PropertyType type,
                                      std::string value) = 0;

        /**
         * @brief Check if a document property has value.
         *
         * @param propID
         * @param objID
         * @param type
         * @return true
         * @return false
         */
        virtual bool exists(int propID, int objID, PropertyType type) = 0;

        /**
         * @brief finds a value object given its prop id and the parent id
         *
         * @param propID
         * @param objID
         * @return optional<int>
         */
        virtual std::optional<int> findObjectId(int propID, int objID) = 0;

        /**
         * @brief Removes all the values associated with a document.
         *
         * @param objID
         */
        virtual void removeAllObject(int objID) = 0;
    };
}  // namespace nldb