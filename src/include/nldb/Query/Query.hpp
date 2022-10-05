#pragma once

#include "nldb/DB/IDB.hpp"
#include "nldb/Implementation.hpp"
#include "nldb/Query/CollectionQuery.hpp"
#include "nldb/Query/QueryPlannerSources.hpp"

namespace nldb {
    template <typename T>
    class Query {
       public:
        Query(T* pConnection) : connection(pConnection) {
            repositories = RepositoriesImpl<T>::create(connection);
        }

        QueryPlannerSources from(const char* collection1Name) {
            auto coll = this->collection(collection1Name);

            // create new repositories, query repositories might be reused.
            auto newCtx = QueryPlannerContext {
                .from = {CollectionQuery(&this->repositories, coll)},
                .repos = RepositoriesImpl<T>::create(connection),
                .queryRunner = QueryRunnerImpl<T>::create(connection)};

            return std::move(newCtx);
        }

        QueryPlannerSources from(CollectionQuery collection1) {
            auto newCtx = QueryPlannerContext {
                .from = {collection1},
                .repos = RepositoriesImpl<T>::create(connection),
                .queryRunner = QueryRunnerImpl<T>::create(connection)};

            return std::move(newCtx);
        }

        /**
         * @brief Get a collection from the db based on the name.
         *
         * @param name
         * @return CollectionQuery
         */
        CollectionQuery collection(const char* name) {
            return CollectionQuery(&this->repositories, name);
        }

       private:
        T* connection;
        Repositories repositories;
    };
}  // namespace nldb