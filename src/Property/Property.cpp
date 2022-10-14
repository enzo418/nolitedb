#include "nldb/Property/Property.hpp"

#include "nldb/Property/AggregatedProperty.hpp"
#include "nldb/Property/PropertyExpression.hpp"
#include "nldb/Property/SortedProperty.hpp"

namespace nldb {

    Property::Property(const std::string& pName,
                       std::optional<std::string> pCollName)
        : name(pName), collName(pCollName) {}

    Property::Property(int pId, const std::string& pName, PropertyType pType,
                       int collID)
        : id(pId), name(pName), type(pType), collectionId(collID) {}

    // getters
    std::string Property::getName() const { return name; }

    PropertyType Property::getType() const { return type; }

    int Property::getId() const { return id; }

    int Property::getCollectionId() const { return collectionId; }

    std::optional<std::string> Property::getParentCollName() {
        return collName;
    }

    bool Property::isParentNameAnExpression() {
        if (!collName) return false;

        return collName->find_first_of(".") != collName->npos;
    }

    // aggregate functions
    AggregatedProperty Property::countAs(const char* alias) {
        return AggregatedProperty(*this, AggregationType::COUNT, alias);
    }

    AggregatedProperty Property::maxAs(const char* alias) {
        return AggregatedProperty(*this, AggregationType::MAX, alias);
    }

    AggregatedProperty Property::minAs(const char* alias) {
        return AggregatedProperty(*this, AggregationType::MIN, alias);
    }

    AggregatedProperty Property::sumAs(const char* alias) {
        return AggregatedProperty(*this, AggregationType::SUM, alias);
    }

    AggregatedProperty Property::averageAs(const char* alias) {
        return AggregatedProperty(*this, AggregationType::AVG, alias);
    }

    // sort functions
    SortedProperty Property::desc() {
        return SortedProperty(*this, SortType::DESC);
    }

    SortedProperty Property::asc() {
        return SortedProperty(*this, SortType::ASC);
    }

    // logical
    PropertyExpression Property::operator>(const LogicConstValue& right) {
        return PropertyExpression(PropertyExpressionOperator::GT, *this, right);
    }

    PropertyExpression Property::operator>=(const LogicConstValue& right) {
        return PropertyExpression(PropertyExpressionOperator::GTE, *this,
                                  right);
    }

    PropertyExpression Property::operator<(const LogicConstValue& right) {
        return PropertyExpression(PropertyExpressionOperator::LT, *this, right);
    }

    PropertyExpression Property::operator<=(const LogicConstValue& right) {
        return PropertyExpression(PropertyExpressionOperator::LTE, *this,
                                  right);
    }

    PropertyExpression Property::operator==(const LogicConstValue& right) {
        return PropertyExpression(PropertyExpressionOperator::EQ, *this, right);
    }

    PropertyExpression Property::operator!=(const LogicConstValue& right) {
        return PropertyExpression(PropertyExpressionOperator::NEQ, *this,
                                  right);
    }

    PropertyExpression Property::operator%(const LogicConstValue& right) {
        return PropertyExpression(PropertyExpressionOperator::LIKE, *this,
                                  right);
    }

    PropertyExpression Property::operator^(const LogicConstValue& right) {
        return PropertyExpression(PropertyExpressionOperator::NLIKE, *this,
                                  right);
    }

}  // namespace nldb