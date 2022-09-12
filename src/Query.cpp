#include "Query.hpp"

Query::Query(const Collection& pCl) : cl(pCl) {}

CollectionQueryFactory::CollectionQueryFactory(const Collection& pCl)
    : cl(pCl) {}

CollectionQueryFactory QueryFactory::create(IDB& ctx,
                                            const std::string& docName) {
    return CollectionQueryFactory(Collection::find(ctx, docName));
}