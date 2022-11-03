#pragma once

#include "DBQueryReader.hpp"
#include "nldb/DB/IDB.hpp"
#include "sqlite/sqlite3.h"

namespace nldb {

    class DBSL3 : public IDB {
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

        ~DBSL3();

       private:
        sqlite3* db;
    };
}  // namespace nldb