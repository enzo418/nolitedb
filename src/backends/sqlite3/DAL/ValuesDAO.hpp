#pragma once

#include "nldb/DAL/IValuesDAO.hpp"
#include "nldb/DB/IDB.hpp"

namespace nldb {
    class ValuesDAO : public IValuesDAO {
       public:
        ValuesDAO(IDB* connection);

       public:
        void addStringLike(int propID, int docID, PropertyType type,
                           std::string value) override;

        void addObject(int propID, int docID, int subCollID,
                       int subDocId) override;

        void updateStringLike(int propID, int docID, PropertyType type,
                              std::string value) override;

        void updateObject(int propID, int docID, int subCollID = -1,
                          int subDocId = -1) override;

        bool exists(int propID, int docID, PropertyType type) override;

        std::optional<ValueObjectMapped> findObject(int propID,
                                                    int docID) override;

        void removeAllFromDocument(int docID) override;

       private:
        IDB* conn;
    };
}  // namespace nldb