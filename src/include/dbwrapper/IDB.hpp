#pragma once

#include <memory>
#include <optional>
#include <string>

#include "ParamsBind.hpp"
#include "dbwrapper/IDBQueryReader.hpp"

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
     * if using sqlite, always use ?,:,@,$ before the parameter id in both, the
     * query and the key value passed in this argument.
     *
     * e.g. (insert into "a" values (@num), {{"@num", 42}})
     * @return std::unique_ptr<IDBQueryReader>
     */
    virtual std::unique_ptr<IDBQueryReader> executeReader(
        const std::string& query, const Paramsbind& params) = 0;

    /**
     * @brief Is a convenience wrapper around execute reader.
     * Executes the query handles the steps and the reader life cicle.
     * Generally used for everything else than a select.
     * Params are still bind through the db api.
     *
     * for sqlite3:
     *  - multiple insert values: insert into ... values (,,), (,,,)
     *  - multiple tables, inserts, delete: call the function below, it doesn't
     *    work with this one.
     * @param query
     * @param params
     * @return int rows affected
     */
    virtual int executeOneStep(const std::string& query,
                               const Paramsbind& params) = 0;

    /**
     * @brief Same as above but assures that all the statements
     * will be executed. This is because some db engines doesn't support
     * multiple create, insert statements in a single query.
     *
     * have in mind that the parameters will be inserted in the query without
     * major modifications so it can be insecure.
     *
     * @param query
     * @param params
     * @return int
     */
    virtual int executeMultipleOnOneStepRaw(const std::string& query,
                                            const Paramsbind& params) = 0;

    /**
     * @brief Executes the query and returns the first column as int.
     * Given for convenience since is a common task to do.
     * @param query
     * @param params same as above
     * @return std::optional<int>
     */
    virtual std::optional<int> executeAndGetFirstInt(
        const std::string& query, const Paramsbind& params) = 0;

    /**
     * @brief Get the Last Inserted Row Id.
     *
     * @return int
     */
    virtual int getLastInsertedRowId() = 0;

    /**
     * @brief Get the number of rows affected by the last query.
     *
     * @return int
     */
    virtual std::optional<int> getChangesCount() = 0;

    virtual void throwLastError() = 0;
};