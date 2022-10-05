#include "nldb/Query/CollectionQuery.hpp"

#include "nldb/DAL/Repositories.hpp"

namespace nldb {
    CollectionQuery::CollectionQuery(Repositories* repos,
                                     const std::string& name, std::string alias)
        : Collection(-1, name), repositories(repos) {}

    CollectionQuery::CollectionQuery(Repositories* repos, Collection col,
                                     std::string alias)
        : Collection(std::move(col)), repositories(repos) {}

    CollectionQuery& CollectionQuery::as(std::string alias) {
        this->alias = alias;
        return *this;
    }

    void CollectionQuery::setCollection(const Collection& c) {
        this->id = c.getId();
        this->name = c.getName();
    }

    std::optional<std::string> CollectionQuery::getAlias() const {
        return alias;
    }

    bool CollectionQuery::hasCollectionAssigned() const {
        return this->id != -1;
    }

}  // namespace nldb