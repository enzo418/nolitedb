#include "dbwrapper/sq3wrapper/DB.hpp"

#include <memory>
#include <stdexcept>

bool DBSL3::open(const std::string& path) {
    int rc = sqlite3_open(path.c_str(), &this->db);
    if (rc != SQLITE_OK) {
        this->close();

        return false;
    }
    return true;
}

bool DBSL3::close() {
    if (this->db != nullptr) {
        int rc = sqlite3_close(this->db);

        if (rc != SQLITE_OK) {
            return false;
        }
    }

    return true;
}

std::unique_ptr<IDBQueryReader> DBSL3::execute(const std::string& query,
                                               const Paramsbind& params) {
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0);
    if (rc == SQLITE_OK) {
        return std::make_unique<DBQueryReaderSL3>(this, stmt);
    } else {
        auto msg =
            "Failed to execute statement: " + std::string(sqlite3_errmsg(db));

        throw std::runtime_error(msg);
    }
}

int DBSL3::getLastInsertedRowId() { return sqlite3_last_insert_rowid(db); }

void DBSL3::throwLastError() {
    throw std::runtime_error(sqlite3_errmsg(this->db));
}
