#pragma once

#include "nldb/Property/ComposedProperty.hpp"

#include <stdexcept>
#include <string>
#include <vector>

#include "nldb/DAL/Repositories.hpp"
#include "nldb/Property/Property.hpp"
#include "nldb/Utils/ComposedPropertyParser.hpp"

namespace nldb {
    ComposedProperty::ComposedProperty(Property prop, int pParentCollID,
                                       int pSubCollID,
                                       std::vector<SubProperty>&& pProperties,
                                       Repositories* pRepos)
        : prop(std::move(prop)),
          parentCollectionID(pParentCollID),
          subCollectionID(pSubCollID),
          properties(std::move(pProperties)),
          repos(pRepos) {}

    ComposedProperty::ComposedProperty(
        Property prop, int pParentCollID, int pSubCollID,
        const std::vector<SubProperty>& pProperties, Repositories* pRepos)
        : prop(std::move(prop)),
          parentCollectionID(pParentCollID),
          subCollectionID(pSubCollID),
          properties(pProperties),
          repos(pRepos) {}

    Property ComposedProperty::getProperty() const { return prop; }

    int ComposedProperty::getParentCollectionId() const {
        return parentCollectionID;
    }

    int ComposedProperty::getSubCollectionId() const { return subCollectionID; }

    std::vector<SubProperty>& ComposedProperty::getPropertiesRef() {
        return properties;
    }

    std::vector<SubProperty> ComposedProperty::getProperties() const {
        return properties;
    }

    void ComposedProperty::addProperty(SubProperty prop) {
        this->properties.push_back(std::move(prop));
    }

    ComposedProperty ComposedProperty::evaluateExpresion(
        int collID, Repositories* repos,
        const ComposedPropertyExpression& expr) {
        return utils::readComposedProperty(std::string(expr.expr, expr.size),
                                           collID, repos);
    }

    Property ComposedProperty::operator[](const char* expr) {
        // instead of throwing, we could use a Property::zombie or empty
        if (repos == nullptr) {
            // Composed property has no real collection associated, how do you
            // expect to get an actual property?
            throw std::runtime_error("Composed property is empty");
        }

        return utils::readProperty(std::string(expr), this->subCollectionID,
                                   this->repos);
    }

    bool ComposedProperty::isEmpty() { return flag_empty; }

    ComposedProperty ComposedProperty::empty() {
        auto c = ComposedProperty(Property(-1, "", PropertyType::OBJECT, -1),
                                  -1, -1, {}, nullptr);
        c.flag_empty = true;
        return c;
    }
}  // namespace nldb