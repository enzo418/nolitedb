#pragma once

#include <string>
#include <vector>

#include "nldb/nldb_json.hpp"

namespace nldb {
    class IRepositoryDocument {
       public:
        /**
         * @brief Adds a document and return the id.
         *
         * @param collectionID
         * @return int
         */
        virtual int add(int collectionID) = 0;

        virtual void remove(int id) = 0;

        virtual bool exists(int id) = 0;
    };
}  // namespace nldb