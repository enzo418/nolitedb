#include "nldb/Property/Property.hpp"

#include "nldb/Common.hpp"
#include "nldb/LOG/log.hpp"
#include "nldb/Property/AggregatedProperty.hpp"
#include "nldb/Property/PropertyExpression.hpp"
#include "nldb/Property/SortedProperty.hpp"

namespace nldb {

    Property::Property(const std::string& coll_name)
        : name(coll_name), type(PropertyType::OBJECT) {}

    Property::Property(const std::string& pName,
                       std::optional<std::string> pCollName)
        : collName(pCollName), name(pName) {
        if (pName == common::internal_id_string) {
            type = PropertyType::ID;
        } else {
            // it doesn't matter to which type we set it because we later try to
            // find it by its name, but is useful that it won't be an id by
            // default
            type = PropertyType::STRING;
        }
#ifdef NLDB_SHOW_ID_WARNING
        if (pName != common::internal_id_string && pName == "id") {
            NLDB_WARN(
                "If you are trying to get the document id, use the "
                "internal id '{}' instead of 'id'",
                common::internal_id_string);
        }
#endif
    }

    Property::Property(snowflake pId, const std::string& pName,
                       PropertyType pType, snowflake collID)
        : name(pName), type(pType), id(pId), collectionId(collID) {}

    // getters
    std::string Property::getName() const { return name; }

    PropertyType Property::getType() const { return type; }

    snowflake Property::getId() const { return id; }

    snowflake Property::getCollectionId() const { return collectionId; }

    std::optional<std::string> Property::getParentCollName() {
        return collName;
    }

    bool Property::isParentNameAnExpression() {
        if (!collName) return false;

        return collName->find_first_of(".") != collName->npos;
    }

    Property Property::operator[](const std::string& pName) {
        const std::string thisPropParent =
            this->getParentCollName() ? this->getParentCollName().value() + "."
                                      : "";

        return Property(pName, thisPropParent + this->name);
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