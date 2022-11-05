#pragma once

#include "nldb/DB/IDB.hpp"
#include "nldb/Property/Property.hpp"
#include "nldb/Utils/ValueBuffer.hpp"
#include "nldb/typedef.hpp"

namespace nldb {
    struct BufferValueStringLike {
        snowflake propID;
        snowflake objID;
        PropertyType type;
        std::string value;
    };

    struct BufferValueDependentObject {
        snowflake id;
        snowflake prop_id;
        snowflake obj_id;
    };

    struct BufferValueIndependentObject {
        snowflake id;
        snowflake prop_id;
    };

    struct BufferValueRootProperty {
        snowflake id;
        std::string name;
    };

    struct BufferValueProperty {
        snowflake id;
        std::string name;
        snowflake coll_id;
        PropertyType type;
    };

    struct BufferValueCollection {
        snowflake id;
        std::string name;
        snowflake owner_id;
    };

    struct BufferData {
        BufferData(IDB* db, int SmallBufferSize, int MediumBufferSize,
                   int LargeBufferSize);

        /**
         * @brief if there is data pending to be sent, make sure it is sent.
         */
        void pushPendingData();

        /**
         * @brief add a value to the buffer, it might flush if there is not
         * enough space.
         *
         * @param val value to add
         */
        void add(const BufferValueCollection& val);
        void add(const BufferValueRootProperty& val);
        void add(const BufferValueProperty& val);
        void add(const BufferValueStringLike& val);
        void add(const BufferValueDependentObject& val);
        void add(const BufferValueIndependentObject& val);

        virtual ~BufferData();

       protected:
        template <typename T>
        void inline _add(const T& val, Buffer<T>& buff) {
            if (!buff.Add(val)) {
                this->pushPendingData();
                buff.Add(val);
            }
        }

       protected:
        // Property collection
        Buffer<BufferValueCollection> bufferCollection;

        // Property repo
        Buffer<BufferValueRootProperty> bufferRootProperty;
        Buffer<BufferValueProperty> bufferProperty;

        // Values DAO
        Buffer<BufferValueStringLike> bufferStringLike;
        Buffer<BufferValueDependentObject> bufferDependentObject;
        Buffer<BufferValueIndependentObject> bufferIndependentObject;

        NullLock lock;

        IDB* conn;

       private:
        virtual void pushRootProperties() = 0;
        virtual void pushCollections() = 0;
        virtual void pushProperties() = 0;
        virtual void pushIndependentObjects() = 0;
        virtual void pushDependentObjects() = 0;
        virtual void pushStringLikeValues() = 0;
    };
}  // namespace nldb