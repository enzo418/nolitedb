#pragma once

#include "nldb/Property/ComposedProperty.hpp"

#include <string>
#include <vector>

#include "nldb/Utils/ComposedPropertyParser.hpp"

namespace nldb {
    ComposedProperty::ComposedProperty(Property prop, int pParentCollID,
                                       int pSubCollID,
                                       std::vector<SubProperty>&& pProperties)
        : prop(std::move(prop)),
          parentCollectionID(pParentCollID),
          subCollectionID(pSubCollID),
          properties(std::move(pProperties)) {}

    ComposedProperty::ComposedProperty(
        Property prop, int pParentCollID, int pSubCollID,
        const std::vector<SubProperty>& pProperties)
        : prop(std::move(prop)),
          parentCollectionID(pParentCollID),
          subCollectionID(pSubCollID),
          properties(pProperties) {}

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

    ComposedProperty ComposedProperty::empty(Property prop) {
        return ComposedProperty(std::move(prop), prop.getCollectionId(), -1,
                                {});
    }
}  // namespace nldb