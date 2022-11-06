#pragma once

#include <memory>
#include <optional>
#include <string>

#include "ParameterBinder.hpp"
#include "nldb/DB/IDBQueryReader.hpp"
#include "nldb/typedef.hpp"

namespace nldb {
    class IDB {
       public:
        /**
         * @brief Open a database and creates the nosql tables.
         *
         * @param path
         * @return true
         * @return false
         */
        virtual bool open(const std::string& path) = 0;

        virtual bool close() = 0;

        /**
         * @brief executes a query in the database.
         *
         * @param query query
         * @param params values to bind in the query.
         * if using sqlite, always use ?,:,@,$ before the parameter id in both,
         * the query and the key value passed in this argument.
         *
         * e.g. (insert into "a" values (@num), {{"@num", 42}})
         * @return std::unique_ptr<IDBQueryReader>
         */
        virtual std::unique_ptr<IDBQueryReader> executeReader(
            const std::string& query, const Paramsbind& params) = 0;

        /**
         * @brief Starts a transaction.
         */
        virtual void begin() = 0;

        /**
         * @brief Commits the transaction.
         */
        virtual void commit() = 0;

        /**
         * @brief Rollback the transaction.
         */
        virtual void rollback() = 0;

        /**
         * @brief Executes a query immediately (if no transaction was started).
         * Should support multiple inserts.
         *
         * @param query
         * @param params
         * @return void
         */
        virtual void execute(const std::string& query,
                             const Paramsbind& params) = 0;

        /**
         * @brief Executes the query and returns the first column as int.
         * Given for convenience since is a common task to do.
         * @param query
         * @param params same as above
         * @return std::optional<snowflake>
         */
        virtual std::optional<snowflake> executeAndGetFirstInt(
            const std::string& query, const Paramsbind& params) = 0;

        /**
         * @brief Get the Last Inserted Row Id.
         *
         * @return snowflake id
         */
        virtual snowflake getLastInsertedRowId() = 0;

        /**
         * @brief Get the number of rows affected by the last query.
         *
         * @return int
         */
        virtual std::optional<int> getChangesCount() = 0;

        virtual void throwLastError() = 0;

        /**
         * @brief Logs the current database status, mostly about the memory
         * usage.
         *
         */
        virtual void logStatus() = 0;

        virtual ~IDB() = default;
    };
}  // namespace nldb