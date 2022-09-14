#include "PropertyRep.hpp"

#include <optional>
#include <stdexcept>
#include <variant>

#include "Enums.hpp"
#include "dbwrapper/ParamsBind.hpp"
#include "logger/Logger.h"

PropertyRep::PropertyRep(const std::string& pName, int pId, PropertyType pType)
    : name(pName), id(pId), type(pType) {}

SqlStatement<std::string> getStatement(PropertyRep* lf, Operator op,
                                       PropertyRep& rt) {
    return SqlStatement<std::string>(lf->getStatement() + " " +
                                     OperatorToString(op) + " " +
                                     rt.getStatement());
}

SqlStatement<std::string> getStatement(PropertyRep* lf, Operator op,
                                       const std::string& rt) {
    return SqlStatement<std::string>(lf->getStatement() + ".value " +
                                     OperatorToString(op) + " " + rt);
}

SqlStatement<std::string> getStatement(PropertyRep* lf, Operator op,
                                       const char* rt) {
    return SqlStatement<std::string>(lf->getStatement() + ".value " +
                                     OperatorToString(op) + " " + rt);
}

int PropertyRep::getId() const { return id; }

PropertyType PropertyRep::getType() const { return type; }
std::string_view PropertyRep::getName() const { return name; }

std::string PropertyRep::getStatement() const {
    switch (this->type) {
        case PropertyType::INTEGER:
            return this->name + "_vi";
        case PropertyType::DOUBLE:
            return this->name + "_vd";
        case PropertyType::STRING:
            return this->name + "_vs";
        default:
            return this->name;
    }
}

std::string PropertyRep::getTableNameForTypeValue(PropertyType type) {
    switch (type) {
        case PropertyType::INTEGER:
            return "value_int";
        case PropertyType::DOUBLE:
            return "value_double";
        case PropertyType::STRING:
            return "value_string";
        default:
            throw std::runtime_error("type not supported");
    }
}

std::optional<PropertyRep> PropertyRep::find(IDB* ctx, int collectionID,
                                             const std::string& name) {
    // else check db
    auto reader = ctx->executeReader(
        "SELECT id, type FROM property where coll_id = @colid and name = @name",
        {{"@colid", collectionID}, {"@name", name}});

    int id;
    int type;

    std::shared_ptr<IDBRowReader> row;
    if (reader->readRow(row)) {
        id = row->readInt32(0);
        type = row->readInt32(1);

        return PropertyRep(name, id, (PropertyType)type);
    } else {
        LogWarning("Missing property from col %i with name %s", collectionID,
                   name.c_str());
        return std::nullopt;
    }
}

std::string getValAsString(const RightValue& val) {
    if (std::holds_alternative<int>(val)) {
        return std::to_string(std::get<int>(val));
    } else if (std::holds_alternative<double>(val)) {
        return std::to_string(std::get<double>(val));
    } else if (std::holds_alternative<std::string>(val)) {
        return utils::paramsbind::encloseQuotesConst(
            std::get<std::string>(val));
    } else if (std::holds_alternative<const char*>(val)) {
        return utils::paramsbind::encloseQuotesConst(
            std::get<const char*>(val));
    } else if (std::holds_alternative<PropertyRep>(val)) {
        return std::get<PropertyRep>(val).getStatement() + ".value";
    } else {
        throw std::runtime_error("Type not supported");
    }
}

SqlStatement<std::string> PropertyRep::operator<(RightValue rt) {
    return SqlStatement<std::string>(this->getStatement() + ".value" +
                                     OperatorToString(Operator::LT) +
                                     getValAsString(rt));
}

SqlStatement<std::string> PropertyRep::operator<=(RightValue rt) {
    return SqlStatement<std::string>(this->getStatement() + ".value" +
                                     OperatorToString(Operator::LTE) +
                                     getValAsString(rt));
}

SqlStatement<std::string> PropertyRep::operator>(RightValue rt) {
    return SqlStatement<std::string>(this->getStatement() + ".value" +
                                     OperatorToString(Operator::GT) +
                                     getValAsString(rt));
}

SqlStatement<std::string> PropertyRep::operator>=(RightValue rt) {
    return SqlStatement<std::string>(this->getStatement() + ".value" +
                                     OperatorToString(Operator::GTE) +
                                     getValAsString(rt));
}

// EQUAL
SqlStatement<std::string> PropertyRep::operator==(RightValue rt) {
    return SqlStatement<std::string>(this->getStatement() + ".value" +
                                     OperatorToString(Operator::EQ) +
                                     getValAsString(rt));
}

// NOT EQUAL
SqlStatement<std::string> PropertyRep::operator!=(RightValue rt) {
    return SqlStatement<std::string>(this->getStatement() + ".value" +
                                     OperatorToString(Operator::NEQ) +
                                     getValAsString(rt));
}

// LIKE
SqlStatement<std::string> PropertyRep::operator%(RightValue rt) {
    return SqlStatement<std::string>(this->getStatement() + ".value" + " " +
                                     OperatorToString(Operator::LIKE) + " " +
                                     getValAsString(rt));
}

// NOT LIKE
SqlStatement<std::string> PropertyRep::operator^(RightValue rt) {
    return SqlStatement<std::string>(this->getStatement() + ".value" + " " +
                                     OperatorToString(Operator::NLIKE) + " " +
                                     getValAsString(rt));
}