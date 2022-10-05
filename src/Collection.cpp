#include "nldb/Collection.hpp"

namespace nldb {
    Collection::Collection(int id, const std::string& name)
        : id(id), name(name) {}

    int Collection::getId() const { return id; }

    std::string Collection::getName() const { return name; }
}  // namespace nldb