#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace nldb {
    class IDBRowReader {
       public:
        virtual std::string readString(uint16_t i) = 0;
        virtual int64_t readInt64(uint16_t i) = 0;
        virtual int readInt32(uint16_t i) = 0;
        virtual double readDouble(uint16_t i) = 0;
        virtual bool readBoolean(uint16_t i) = 0;
        virtual bool isNull(uint16_t i) = 0;

        virtual ~IDBRowReader() = default;
    };

    class IDBQueryReader {
       public:
        virtual bool readRow(std::shared_ptr<IDBRowReader>& row) = 0;

        virtual ~IDBQueryReader() = default;
    };
}  // namespace nldb