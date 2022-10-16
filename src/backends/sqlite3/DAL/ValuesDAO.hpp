#pragma once

#include "nldb/DAL/IValuesDAO.hpp"
#include "nldb/DB/IDB.hpp"
#include "nldb/Utils/ValueBuffer.hpp"

namespace nldb {

    struct BufferValue {
        int propID;
        int objID;
        PropertyType type;
        std::string value {""};
    };

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

        bool existsObject(int objID) override;

        std::optional<int> findObjectId(int propID, int objID) override;

        void removeObject(int objID) override;

        void deferAddStringLike(int propID, int objID, PropertyType type,
                                std::string value) override;

        void pushPendingData() override;

        ~ValuesDAO();

       private:
        IDB* conn;
        Buffer<BufferValue> buffer;
    };
}  // namespace nldb