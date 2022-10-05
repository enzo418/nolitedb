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
         * @brief Adds a new value to a document property.
         *
         * @param propID
         * @param docID
         * @param type any type but OBJECT is allowed.
         * @param value
         */
        virtual void addStringLike(int propID, int docID, PropertyType type,
                                   std::string value) = 0;

        /**
         * @brief adds a new value object to a document property.
         *
         * @param propID
         * @param docID
         * @param subCollID
         * @param subDocId
         */
        virtual void addObject(int propID, int docID, int subCollID,
                               int subDocId) = 0;

        /**
         * @brief Updates a document property value.
         *
         * @param propID
         * @param docID
         * @param type any type but OBJECT is allowed.
         * @param value
         */
        virtual void updateStringLike(int propID, int docID, PropertyType type,
                                      std::string value) = 0;

        /**
         * @brief updates a value of type object
         *
         * @param propID
         * @param docID
         * @param subCollID new sub collection id or -1 to not update
         * @param subDocId new document id or -1 to not update
         */
        virtual void updateObject(int propID, int docID, int subCollID = -1,
                                  int subDocId = -1) = 0;

        /**
         * @brief Check if a document property has value.
         *
         * @param propID
         * @param docID
         * @param type
         * @return true
         * @return false
         */
        virtual bool exists(int propID, int docID, PropertyType type) = 0;

        /**
         * @brief finds a value of type object.
         *
         * @param propID
         * @param docID
         * @return ValueObjectMapped
         */
        virtual std::optional<ValueObjectMapped> findObject(int propID,
                                                            int docID) = 0;

        /**
         * @brief Removes all the values associated with a document.
         *
         * @param docID
         */
        virtual void removeAllFromDocument(int docID) = 0;
    };
}  // namespace nldb