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

    Property Collection::operator[](const std::string& expr) {
        auto pos = expr.find_last_of('.');
        if (pos != expr.npos) {
            // later we parse the expression and search for the child collection
            std::string propName = expr.substr(pos + 1, expr.length() - pos);
            std::string parentExpr = expr.substr(0, pos);
            return Property(propName, this->name + "." + parentExpr);
        } else {
            return Property(expr, this->name);
        }
    }
}  // namespace nldb