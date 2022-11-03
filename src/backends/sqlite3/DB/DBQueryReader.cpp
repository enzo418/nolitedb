#include "DBQueryReader.hpp"

#include <iostream>
#include <memory>
#include <stdexcept>

namespace nldb {

    DBRowReaderSL3::DBRowReaderSL3(IDB* pDb, sqlite3_stmt* pStmt)
        : stmt(pStmt), db(pDb) {}

    std::string DBRowReaderSL3::readString(uint16_t i) {
        auto str = sqlite3_column_text(stmt, i);
        if (str != nullptr)
            return std::string(reinterpret_cast<const char*>(str));
        throw std::runtime_error("Missing column");
    }

    int64_t DBRowReaderSL3::readInt64(uint16_t i) {
        return sqlite3_column_int64(stmt, i);
    }

    int DBRowReaderSL3::readInt32(uint16_t i) {
        return sqlite3_column_int(stmt, i);
    }

    double DBRowReaderSL3::readDouble(uint16_t i) {
        return sqlite3_column_double(stmt, i);
    }

    bool DBRowReaderSL3::readBoolean(uint16_t i) { return readInt32(i) == 1; }

    bool DBRowReaderSL3::isNull(uint16_t i) {
        return sqlite3_column_type(stmt, i) == SQLITE_NULL;
    }

    // ---
    bool DBQueryReaderSL3::readRow(std::shared_ptr<IDBRowReader>& row) {
        if (this->allWasRead) return false;

        if (!row) {
            row = std::make_shared<DBRowReaderSL3>(this->db, this->stmt);
        }

        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            return true;
        } else if (rc == SQLITE_DONE) {
            this->allWasRead = true;
            return false;
        } else {
            this->db->throwLastError();
            return false;
        }
    }

    DBQueryReaderSL3::DBQueryReaderSL3(IDB* pDb, sqlite3_stmt* pStmt)
        : stmt(pStmt), db(pDb) {}

    DBQueryReaderSL3::~DBQueryReaderSL3() { sqlite3_finalize(stmt); }
}  // namespace nldb