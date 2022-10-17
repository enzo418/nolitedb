#pragma once

#include <memory>

#include "DAL/Repositories.hpp"
#include "DB/IDB.hpp"
#include "Implementation.hpp"
#include "backends/sqlite3/DAL/BufferDataSQ3.hpp"
#include "backends/sqlite3/DAL/RepositoryCollection.hpp"
#include "backends/sqlite3/DAL/RepositoryProperty.hpp"
#include "backends/sqlite3/DAL/ValuesDAO.hpp"
#include "backends/sqlite3/DB/DB.hpp"
#include "backends/sqlite3/Query/QueryRunner.hpp"
#include "nldb/DAL/Buffer/BufferedRepositoryCollection.hpp"
#include "nldb/DAL/Buffer/BufferedRepositoryProperty.hpp"
#include "nldb/DAL/Buffer/BufferedValuesDAO.hpp"
#include "nldb/DAL/Cache/CachedRepositoryCollection.hpp"
#include "nldb/DAL/Cache/CachedRepositoryPropertyCache.hpp"

/**
 * This is the default implementation for the sqlite 3 backend.
 * Include this file to use this implementation.
 */

namespace nldb {
    template <>
    struct RepositoriesImpl<DBSL3> {
        static std::shared_ptr<Repositories> create(IDB* conn) {
            auto bufferData = std::make_shared<BufferDataSQ3>(conn);

            return std::make_shared<Repositories>(
                // std::make_unique<RepositoryCollection>(conn),
                // std::make_unique<RepositoryProperty>(conn),

                std::make_unique<CachedRepositoryCollection>(
                    conn,
                    std::make_unique<BufferedRepositoryCollection>(
                        conn, std::make_unique<RepositoryCollection>(conn),
                        bufferData)),

                std::make_unique<CachedRepositoryProperty>(
                    conn, std::make_unique<BufferedRepositoryProperty>(
                              conn, std::make_unique<RepositoryProperty>(conn),
                              bufferData)),

                std::make_unique<BufferedValuesDAO>(
                    conn, std::make_unique<ValuesDAO>(conn), bufferData),
                bufferData);
        }
    };

    template <>
    struct QueryRunnerImpl<DBSL3> {
        static std::unique_ptr<QueryRunner> create(
            IDB* conn, std::shared_ptr<Repositories>& repos) {
            return std::make_unique<QueryRunnerSQ3>(conn, repos);
        }
    };
}  // namespace nldb