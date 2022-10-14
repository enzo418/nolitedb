#pragma once

#include <memory>

#include "nldb/DAL/Repositories.hpp"
#include "nldb/DB/IDB.hpp"
#include "nldb/Implementation.hpp"
#include "nldb/Query/QueryPlannerSources.hpp"

namespace nldb {
    template <typename T>
    class Query {
       public:
        /**
         * @brief Construct a new Query object.
         * No cache will be reused.
         *
         * @param pConnection
         */
        Query(T* pConnection) : connection(pConnection) {
            // having the repositories initialized here and passing it to every
            // query runner instance gives us a better cache.
            repositories = RepositoriesImpl<T>::create(connection);
        }

        /**
         * @brief Construct a new Query object from a DB connection and
         * repository instance. Use this overload to reuse the repositories
         * cache.
         *
         * @param pConnection
         * @param pRepos
         */
        Query(T* pConnection, std::shared_ptr<Repositories> pRepos)
            : connection(pConnection), repositories(pRepos) {}

        QueryPlannerSources from(const char* collection1Name) {
            auto newCtx =
                QueryPlannerContext {.from = {Collection(collection1Name)},
                                     .queryRunner = QueryRunnerImpl<T>::create(
                                         connection, repositories)};

            return std::move(newCtx);
        }

        QueryPlannerSources from(Collection collection1) {
            auto newCtx =
                QueryPlannerContext {.from = {collection1},
                                     .queryRunner = QueryRunnerImpl<T>::create(
                                         connection, repositories)};

            return std::move(newCtx);
        }

        /**
         * @brief Get a collection from the db based on the name.
         *
         * @param name
         * @return Collection
         */
        Collection collection(const char* name) { return Collection(name); }

       private:
        T* connection;
        std::shared_ptr<Repositories> repositories;
    };
}  // namespace nldb