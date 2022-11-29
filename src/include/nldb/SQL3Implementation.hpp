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
#include "nldb/DAL/BufferData.hpp"
#include "nldb/DAL/Cache/CachedRepositoryCollection.hpp"
#include "nldb/DAL/Cache/CachedRepositoryPropertyCache.hpp"
#include "nldb/DAL/IRepositoryCollection.hpp"
#include "nldb/DAL/IValuesDAO.hpp"
#include "nldb/Query/Query.hpp"

#ifdef NLDB_INSTALL
#include "nldb_config.h"
#endif

/**
 * This is the default implementation for the sqlite 3 backend.
 * Include this file to use this implementation.
 */

namespace nldb {
    template <>
    struct RepositoriesImpl<DBSL3> {
        static std::shared_ptr<Repositories> create(
            IDB* conn, const QueryConfiguration& cfg) {
            //
            std::shared_ptr<BufferData> bufferData =
                cfg.PreferBuffer
                    ? std::make_shared<BufferDataSQ3>(conn, cfg.SmallBufferSize,
                                                      cfg.MediumBufferSize,
                                                      cfg.LargeBufferSize)
                    : nullptr;

            // build repositories
            std::unique_ptr<IRepositoryCollection> repoColl =
                std::make_unique<RepositoryCollection>(conn);

            std::unique_ptr<IRepositoryProperty> repoProp =
                std::make_unique<RepositoryProperty>(conn);

            std::unique_ptr<IValuesDAO> valuesDao =
                std::make_unique<ValuesDAO>(conn);

            if (cfg.PreferBuffer) {
                repoColl = std::make_unique<BufferedRepositoryCollection>(
                    std::move(repoColl), bufferData);

                repoProp = std::make_unique<BufferedRepositoryProperty>(
                    std::move(repoProp), bufferData);

                valuesDao = std::make_unique<BufferedValuesDAO>(
                    std::move(valuesDao), bufferData);
            }

            if (cfg.PreferCache) {
                repoColl = std::make_unique<CachedRepositoryCollection>(
                    std::move(repoColl));

                repoProp = std::make_unique<CachedRepositoryProperty>(
                    std::move(repoProp));
            }

            return std::make_shared<Repositories>(
                std::move(repoColl), std::move(repoProp), std::move(valuesDao),
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