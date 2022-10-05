#pragma once

#include <memory>

#include "DAL/Repositories.hpp"
#include "DB/IDB.hpp"
#include "Implementation.hpp"
#include "backends/sqlite3/DAL/RepositoryCollection.hpp"
#include "backends/sqlite3/DAL/RepositoryDocument.hpp"
#include "backends/sqlite3/DAL/RepositoryProperty.hpp"
#include "backends/sqlite3/DAL/ValuesDAO.hpp"
#include "backends/sqlite3/DB/DB.hpp"
#include "backends/sqlite3/Query/QueryRunner.hpp"

/**
 * This is the default implementation for the sqlite 3 backend.
 * Include this file to use this implementation.
 */

namespace nldb {
    template <>
    struct RepositoriesImpl<DBSL3> {
        static Repositories create(IDB* conn) {
            return Repositories {
                .repositoryCollection =
                    std::make_unique<RepositoryCollection>(conn),
                .repositoryDocument =
                    std::make_unique<RepositoryDocument>(conn),
                .repositoryProperty =
                    std::make_unique<RepositoryProperty>(conn),
                .valuesDAO = std::make_unique<ValuesDAO>(conn)};
        }
    };

    template <>
    struct QueryRunnerImpl<DBSL3> {
        static std::unique_ptr<QueryRunner> create(IDB* conn) {
            return std::make_unique<QueryRunnerSQ3>(conn);
        }
    };
}  // namespace nldb