#include "dbwrapper/sq3wrapper/DB.hpp"

#include <filesystem>
#include <memory>
#include <optional>
#include <stdexcept>

#include "Utils.hpp"
#include "dbwrapper/IDBQueryReader.hpp"
#include "dbwrapper/ParamsBind.hpp"
#include "dbwrapper/sq3wrapper/DBInitializer.hpp"
#include "logger/Logger.h"

bool DBSL3::open(const std::string& path) {
    bool fileExists = std::filesystem::exists(path);

    int rc = sqlite3_open(path.c_str(), &this->db);
    if (rc != SQLITE_OK) {
        this->close();

        return false;
    }

    if (!fileExists) {
        DBInitializer::createTablesAndFKeys(this);
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

std::unique_ptr<IDBQueryReader> DBSL3::executeReader(const std::string& query,
                                                     const Paramsbind& params) {
    // prepare sql
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0);

    if (rc == SQLITE_OK) {
        // bind params
        for (const auto& it : params) {
            if (!it.first.starts_with("@") && !it.first.starts_with("$") &&
                !it.first.starts_with("?") && !it.first.starts_with(":")) {
                LogWarning("The parameter '%s' doesn't start with @,$,: or ?",
                           it.first.c_str());
            }

            int idx = sqlite3_bind_parameter_index(stmt, it.first.c_str());
            if (std::holds_alternative<int>(it.second)) {
                sqlite3_bind_int(stmt, idx, std::get<int>(it.second));
            } else if (std::holds_alternative<double>(it.second)) {
                sqlite3_bind_double(stmt, idx, std::get<double>(it.second));
            } else if (std::holds_alternative<std::string>(it.second)) {
                sqlite3_bind_text(stmt, idx,
                                  std::get<std::string>(it.second).c_str(), -1,
                                  SQLITE_STATIC);
            } else {
                // variant protects againts this but anyway... in case we change
                // it
                throw std::runtime_error("Type not supported");
            }
        }

        // we are done, now the user can read the results
        return std::make_unique<DBQueryReaderSL3>(this, stmt);
    } else {
        this->throwLastError();
        return nullptr;
    }
}

int DBSL3::executeMultipleOnOneStepRaw(const std::string& query,
                                       const Paramsbind& params) {
    std::string sql(query);

    for (auto& param : params) {
        utils::replaceAllOccurrences(
            sql, param.first,
            utils::paramsbind::getBindValueAsString(param.second, true));
    }

    char* err = 0;
    int rc = sqlite3_exec(db, sql.c_str(), 0, 0, &err);

    if (rc != SQLITE_OK) {
        LogError("Could not execute query, error: %s", err);
        sqlite3_free(err);
        this->throwLastError();
        return -1;  // clangd is blind
    } else {
        return this->getChangesCount().value();
    }
}

int DBSL3::executeOneStep(const std::string& query, const Paramsbind& params) {
    int rowsCount {0};

    auto reader = this->executeReader(query, params);
    std::shared_ptr<IDBRowReader> row;
    while (reader->readRow(row)) {
        ++rowsCount;
    }

    return rowsCount;
}

std::optional<int> DBSL3::executeAndGetFirstInt(const std::string& query,
                                                const Paramsbind& params) {
    auto res = this->executeReader(query, params);
    std::shared_ptr<IDBRowReader> row;
    int first {-1};
    while (res->readRow(row)) {
        return row->readInt32(0);
    }

    return std::nullopt;
}

std::optional<int> DBSL3::getChangesCount() {
    return this->executeAndGetFirstInt("SELECT changes()", {});
}

int DBSL3::getLastInsertedRowId() { return sqlite3_last_insert_rowid(db); }

void DBSL3::throwLastError() {
    throw std::runtime_error(sqlite3_errmsg(this->db));
}
