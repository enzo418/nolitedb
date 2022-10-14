#pragma once

#include "nldb/DAL/IValuesDAO.hpp"
#include "nldb/DB/IDB.hpp"

namespace nldb {
    class ValuesDAO : public IValuesDAO {
       public:
        ValuesDAO(IDB* connection);

       public:
        void addStringLike(int propID, int objID, PropertyType type,
                           std::string value) override;

        int addObject(int propID) override;

        int addObject(int propID, int objID) override;

        void updateStringLike(int propID, int objID, PropertyType type,
                              std::string value) override;

        bool exists(int propID, int objID, PropertyType type) override;

        std::optional<int> findObjectId(int propID, int objID) override;

        void removeAllObject(int objID) override;

       private:
        IDB* conn;
    };
}  // namespace nldb