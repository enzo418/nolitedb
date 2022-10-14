#include "nldb/Collection.hpp"

namespace nldb {
    Collection::Collection(const std::string& pName) : id(-1), name(pName) {}

    Collection::Collection(int id, const std::string& name)
        : id(id), name(name) {}

    int Collection::getId() const { return id; }

    std::string Collection::getName() const { return name; }

    std::optional<std::string> Collection::getAlias() const { return alias; }

    Collection& Collection::as(std::string alias) {
        this->alias = alias;
        return *this;
    }
}  // namespace nldb