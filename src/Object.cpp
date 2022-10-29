#include "nldb/Object.hpp"

#include <stdexcept>
#include <string>
#include <vector>

#include "nldb/DAL/Repositories.hpp"
#include "nldb/Property/Property.hpp"
#include "nldb/Utils/ObjectParser.hpp"

namespace nldb {
    Object::Object(Property prop, std::forward_list<SubProperty>&& pProperties,
                   snowflake pCollId)
        : prop(std::move(prop)),
          properties(std::move(pProperties)),
          collID(pCollId) {}

    Object::Object(Property prop,
                   const std::forward_list<SubProperty>& pProperties,
                   snowflake pCollId)
        : prop(std::move(prop)), properties(pProperties), collID(pCollId) {}

    Property Object::getProperty() const { return prop; }

    Property& Object::getPropertyRef() { return prop; }

    snowflake Object::getCollId() const { return collID; }

    void Object::setCollId(snowflake pCollId) { this->collID = pCollId; }

    std::forward_list<SubProperty>& Object::getPropertiesRef() {
        return properties;
    }

    std::forward_list<SubProperty> Object::getProperties() const {
        return properties;
    }

    SubProperty& Object::addProperty(SubProperty prop) {
        this->properties.push_front(std::move(prop));
        return this->properties.front();
    }

    Object Object::evaluateExpresion(const ObjectExpression& expr,
                                     const std::string& collName) {
        return utils::expandObjectExpression(std::string(expr.expr, expr.size),
                                             collName);
    }

    Property Object::operator[](const std::string& expr) {
        // We can't know how the parent collection name is stored
        // snowflakeernally, imagine this scenario: expr =
        // users.contact.phones.mobile we are sure the first collection name
        // users is users but the database may have choosen to name the
        // subcollection user.contact as __user_contact or r_user_contact. Do we
        // create another Property type? modify its parent member? The easier
        // solution is to make the query runner evaluate the string to get the
        // parent collection.
        std::string parents = this->prop.getParentCollName().has_value()
                                  ? (this->prop.getParentCollName().value() +
                                     "." + this->prop.getName())
                                  : this->prop.getName();
        auto pos = expr.find_last_of('.');
        if (pos != expr.npos) {
            std::string propName = expr.substr(pos + 1, expr.length() - pos);
            std::string parentExpr = expr.substr(0, pos);
            return Property(propName, parents + "." + parentExpr);
        } else {
            return Property(expr, parents);
        }
    }
}  // namespace nldb