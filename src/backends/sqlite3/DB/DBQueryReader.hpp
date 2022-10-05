#pragma once
#include "nldb/DB/IDB.hpp"
#include "nldb/DB/IDBQueryReader.hpp"
#include "sqlite/sqlite3.h"

namespace nldb {
    class DBRowReaderSL3 : public IDBRowReader {
       public:
        DBRowReaderSL3(IDB* db, sqlite3_stmt* stmt);

       public:
        // Note: SQLITE3 - The leftmost column of the result set has the index 0

        std::string readString(uint16_t i) override;
        int64_t readInt64(uint16_t i) override;
        int readInt32(uint16_t i) override;
        double readDouble(uint16_t i) override;
        bool readBoolean(uint16_t i) override;
        bool isNull(uint16_t i) override;

       private:
        IDB* db;
        sqlite3_stmt* stmt;
    };

    class DBQueryReaderSL3 : public IDBQueryReader {
       public:
        DBQueryReaderSL3(IDB* db, sqlite3_stmt* stmt);
        ~DBQueryReaderSL3();

       public:
        bool readRow(std::shared_ptr<IDBRowReader>& row) override;

       private:
        bool allWasRead {false};
        IDB* db;
        sqlite3_stmt* stmt;
    };
}  // namespace nldb