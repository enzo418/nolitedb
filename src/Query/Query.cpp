#include "nldb/Query/Query.hpp"

#include "nldb/Exceptions.hpp"

namespace nldb {
    // Query::Query(IDB* pConnection) : connection(pConnection) {
    //     repositories = this->connection->getRepositoriesInstance();
    // }

    // QueryPlannerSources Query::from(const char* collection1Name) {
    //     auto coll = this->collection(collection1Name);

    //     // create a new repositories object, query might be reused.
    //     return QueryPlannerSources(
    //         {{CollectionQuery(&this->repositories, coll)},
    //          this->connection->getRepositoriesInstance()});
    // }

    // QueryPlannerSources Query::from(CollectionQuery collection1) {
    //     return QueryPlannerSources(QueryPlannerContext {
    //         {collection1}, this->connection->getRepositoriesInstance()});
    // }

    // CollectionQuery Query::collection(const char* name) {
    //     return CollectionQuery(&this->repositories, name);
    // }
}  // namespace nldb