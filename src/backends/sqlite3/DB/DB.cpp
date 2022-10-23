#include "DB.hpp"

#include <filesystem>
#include <memory>
#include <optional>
#include <stdexcept>

#include "DBInitializer.hpp"
#include "nldb/DB/IDBQueryReader.hpp"
#include "nldb/LOG/log.hpp"
#include "nldb/Utils/ParamsBindHelpers.hpp"
#include "nldb/typedef.hpp"

#define SQL3_EXEC_ERR_HNDL(db, SQL_STR, ERROR_MSG)  \
    char* err = 0;                                  \
    int rc = sqlite3_exec(db, SQL_STR, 0, 0, &err); \
    if (rc != SQLITE_OK) {                          \
        NLDB_ERROR(ERROR_MSG ": '{}'", err);        \
        sqlite3_free(err);                          \
        this->throwLastError();                     \
    }

namespace nldb {

    bool DBSL3::open(const std::string& path) {
        NLDB_INFO("OPENING DATABASE: {}", path);

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

    std::unique_ptr<IDBQueryReader> DBSL3::executeReader(
        const std::string& query, const Paramsbind& params) {
        // NLDB_TRACE("Executing: {}", query.c_str());

        if (query.empty()) {
            NLDB_ERROR("Empty query");
            throw std::runtime_error("Empty query");
        }

        // prepare sql
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0);

        if (rc == SQLITE_OK) {
            // bind params
            for (const auto& it : params) {
                if (!it.first.starts_with("@") && !it.first.starts_with("$") &&
                    !it.first.starts_with("?") && !it.first.starts_with(":")) {
                    NLDB_WARN(
                        "The parameter '{}' doesn't start with @,$,: or ?",
                        it.first.c_str());
                }

                int idx = sqlite3_bind_parameter_index(stmt, it.first.c_str());
                if (std::holds_alternative<int>(it.second)) {
                    sqlite3_bind_int(stmt, idx, std::get<int>(it.second));
                } else if (std::holds_alternative<double>(it.second)) {
                    sqlite3_bind_double(stmt, idx, std::get<double>(it.second));
                } else if (std::holds_alternative<std::string>(it.second)) {
                    sqlite3_bind_text(stmt, idx,
                                      std::get<std::string>(it.second).c_str(),
                                      -1, SQLITE_STATIC);
                } else if (std::holds_alternative<snowflake>(it.second)) {
                    sqlite3_bind_int64(stmt, idx,
                                       std::get<snowflake>(it.second));
                } else {
                    // variant protects against this but anyway... in case we
                    // change it
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

    void DBSL3::begin() {
        SQL3_EXEC_ERR_HNDL(db, "BEGIN TRANSACTION;",
                           "Couldn't start the transaction");
    }

    void DBSL3::commit() {
        SQL3_EXEC_ERR_HNDL(db, "END TRANSACTION;",
                           "Couldn't commit the transaction");
    }

    void DBSL3::rollback() {
        SQL3_EXEC_ERR_HNDL(db, "ROLLBACK;",
                           "Couldn't rollback the transaction");
    }

    void DBSL3::execute(const std::string& query, const Paramsbind& params) {
        /**
         * https://www.sqlite.org/lang_transaction.html#deferred_immediate_and_exclusive_transactions
         * To do it in one step use sqlite3_finalize after prepare or simply use
         * sqlite3_exec.
         */
        std::string sql = utils::paramsbind::parseSQL(query, params);

        // NLDB_TRACE("Executing: {}", sql);

        char* err = 0;
        int rc = sqlite3_exec(db, sql.c_str(), 0, 0, &err);

        if (rc != SQLITE_OK) {
            NLDB_ERROR("Could not execute query, error: {}, with query: {}",
                       err, sql);
            sqlite3_free(err);
            this->throwLastError();
        }
    }

    std::optional<snowflake> DBSL3::executeAndGetFirstInt(
        const std::string& query, const Paramsbind& params) {
        auto res = this->executeReader(query, params);
        std::shared_ptr<IDBRowReader> row;
        int first {-1};
        while (res->readRow(row)) {
            return row->readInt64(0);
        }

        return std::nullopt;
    }

    std::optional<int> DBSL3::getChangesCount() {
        return this->executeAndGetFirstInt("SELECT changes();", {});
    }

    snowflake DBSL3::getLastInsertedRowId() {
        return sqlite3_last_insert_rowid(db);
    }

    void DBSL3::throwLastError() {
        throw std::runtime_error(sqlite3_errmsg(this->db));
    }

}  // namespace nldb