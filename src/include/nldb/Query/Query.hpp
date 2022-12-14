#pragma once

#include <memory>

#include "nldb/DAL/Repositories.hpp"
#include "nldb/DB/IDB.hpp"
#include "nldb/Implementation.hpp"
#include "nldb/Query/QueryPlanner.hpp"

namespace nldb {

    struct QueryConfiguration {
        // buffered repositories
        bool PreferBuffer = true;
        uint SmallBufferSize = (int)(5.0 * 1000.0) /* 5 KB */;
        uint MediumBufferSize = (int)(50.0 * 1000.0) /* 50 KB*/;
        uint LargeBufferSize = (int)(1 * 1e6) /* 1 MB */;

        // use cached repositories?
        bool PreferCache = true;

        // reuse cached repositories
        std::shared_ptr<Repositories> cachedRepositories = nullptr;

        // throw if a property is missing (only on select operation)
        // if you use a missing property in a where, the conditions where that
        // property appear will be erased.
        bool ThrowOnSelectMissingProperty = false;
    };

    template <typename T>
    class Query {
       public:
        /**
         * @brief Construct a new Query object.
         * No cache will be reused.
         *
         * @param pConnection
         */
        Query(T* pConnection,
              const QueryConfiguration& pCfg = QueryConfiguration {})
            : connection(pConnection), cfg(pCfg) {
            if (!cfg.cachedRepositories) {
                // having the repositories initialized here and passing it to
                // every query runner instance gives us a better cache.
                repositories = RepositoriesImpl<T>::create(connection, cfg);
            } else {
                repositories = cfg.cachedRepositories;
            }
        }

        QueryPlanner from(const char* collection1Name) {
            auto newCtx =
                QueryPlannerContext {.from = {Collection(collection1Name)},
                                     .queryRunner = QueryRunnerImpl<T>::create(
                                         connection, repositories),
                                     .ThrowOnSelectMissingProperty =
                                         cfg.ThrowOnSelectMissingProperty};

            return std::move(newCtx);
        }

        QueryPlanner from(Collection collection1) {
            auto newCtx =
                QueryPlannerContext {.from = {collection1},
                                     .queryRunner = QueryRunnerImpl<T>::create(
                                         connection, repositories),
                                     .ThrowOnSelectMissingProperty =
                                         cfg.ThrowOnSelectMissingProperty};

            return std::move(newCtx);
        }

        /**
         * @brief Get a collection placeholder.
         *
         * @param name
         * @return Collection
         */
        Collection collection(const char* name) { return Collection(name); }

        std::shared_ptr<Repositories> getRepositories() { return repositories; }

        std::vector<Collection> getCollections() {
            return repositories->repositoryCollection->getAll(true);
        }

       private:
        T* connection;
        std::shared_ptr<Repositories> repositories;
        QueryConfiguration cfg;
    };
}  // namespace nldb