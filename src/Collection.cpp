#include "nldb/Collection.hpp"

namespace nldb {
    Collection::Collection(const std::string& pName) : id(-1), name(pName) {}

    Collection::Collection(snowflake id, const std::string& name)
        : id(id), name(name) {}

    snowflake Collection::getId() const { return id; }

    std::string Collection::getName() const { return name; }

    std::optional<std::string> Collection::getAlias() const { return alias; }

    Collection& Collection::as(std::string alias) {
        this->alias = alias;
        return *this;
    }

    Property Collection::operator[](const std::string& pName) {
        return Property(pName, this->name);
    }
}  // namespace nldb