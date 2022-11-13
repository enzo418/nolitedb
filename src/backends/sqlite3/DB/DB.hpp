#pragma once

#include "DBQueryReader.hpp"
#include "nldb/DB/IDB.hpp"
#include "sqlite/sqlite3.h"

namespace nldb {

    /**
     * @brief Database configuration.
     *
     * For memory configuration check
     * https://www.sqlite.org/malloc.html#pagecache
     *
     */
    struct DBConfig {
        int page_size = 4096;

        int page_cache_size = -1;
        int page_cache_N = -1;
    };

    class DBSL3 : public IDB {
       public:
        DBSL3();
        DBSL3(const DBConfig);

       public:
        bool open(const std::string& path) override;
        bool close() override;
        std::unique_ptr<IDBQueryReader> executeReader(
            const std::string& query, const Paramsbind& params) override;

        void execute(const std::string& query,
                     const Paramsbind& params) override;

        void begin() override;

        void commit() override;

        void rollback() override;

        std::optional<snowflake> executeAndGetFirstInt(
            const std::string& query, const Paramsbind& params) override;

        snowflake getLastInsertedRowId() override;

        std::optional<int> getChangesCount() override;

        void throwLastError() override;

        void logStatus() override;

        ~DBSL3();

       private:
        sqlite3* db;
        DBConfig config;
        void* page_cache_ptr;
    };
}  // namespace nldb