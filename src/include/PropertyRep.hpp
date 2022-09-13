#pragma once
#include <functional>
#include <optional>
#include <string>
#include <type_traits>

#include "CommonTypes.hpp"
#include "Concepts.hpp"
#include "Enums.hpp"
#include "ISqlStatement.hpp"
#include "SqlStatement.hpp"
#include "dbwrapper/IDB.hpp"

class PropertyRep;

template <typename T>
concept StringLikeOrProperty = (StringLike<T> ||
                                std::is_same<PropertyRep, T>::value);

SqlStatement<std::string> getStatement(PropertyRep* lf, Operator op,
                                       PropertyRep& rt);

SqlStatement<std::string> getStatement(PropertyRep* lf, Operator op,
                                       const std::string& rt);

class PropertyRep : public ISqlStatement {
   public:
    PropertyRep(const std::string& pName, int id, PropertyType type);

   public:
    int getId() const;
    PropertyType getType() const;
    std::string_view getName() const;
    std::string getStatement() const override;

    static std::string getTableNameForTypeValue(PropertyType type);

    static std::optional<PropertyRep> find(IDB* ctx, int collectionID,
                                           const std::string& name);

   public:
    SqlStatement<std::string> operator<(PropertyRep& rt) {
        return ::getStatement(this, LT, rt);
    }

    SqlStatement<std::string> operator<=(PropertyRep& rt) {
        return ::getStatement(this, Operator::LTE, rt);
    }

    SqlStatement<std::string> operator>(PropertyRep& rt) {
        return ::getStatement(this, Operator::GT, rt);
    }

    SqlStatement<std::string> operator>=(PropertyRep& rt) {
        return ::getStatement(this, Operator::GTE, rt);
    }

    // EQUAL
    SqlStatement<std::string> operator==(PropertyRep& rt) {
        return ::getStatement(this, Operator::EQ, rt);
    }

    // NOT EQUAL
    SqlStatement<std::string> operator!=(PropertyRep& rt) {
        return ::getStatement(this, Operator::NEQ, rt);
    }

    // LIKE
    SqlStatement<std::string> operator%(PropertyRep& rt) {
        return ::getStatement(this, Operator::LIKE, rt);
    }

    // NOT LIKE
    SqlStatement<std::string> operator^(PropertyRep& rt) {
        return ::getStatement(this, Operator::NLIKE, rt);
    }

   public:  // overloads
    template <typename T>
    SqlStatement<std::string> operator<(T&& rt) {
        return ::getStatement(this, Operator::LT,
                              SqlStatement<T>(rt).getStatement());
    }

    template <typename T>
    SqlStatement<std::string> operator<=(T&& rt) {
        return ::getStatement(this, Operator::LTE,
                              SqlStatement<T>(rt).getStatement());
    }

    template <typename T>
    SqlStatement<std::string> operator>(T&& rt) {
        return ::getStatement(this, Operator::GT,
                              SqlStatement<T>(rt).getStatement());
    }

    template <typename T>
    SqlStatement<std::string> operator>=(T&& rt) {
        return ::getStatement(this, Operator::GTE,
                              SqlStatement<T>(rt).getStatement());
    }

    // EQUAL
    template <typename T>
    SqlStatement<std::string> operator==(T&& rt) {
        return ::getStatement(this, Operator::EQ,
                              SqlStatement<T>(rt).getStatement());
    }

    // NOT EQUAL
    template <typename T>
    SqlStatement<std::string> operator!=(T&& rt) {
        return ::getStatement(this, Operator::NEQ,
                              SqlStatement<T>(rt).getStatement());
    }

    // LIKE
    template <StringLike T>
    SqlStatement<std::string> operator%(T&& rt) {
        return ::getStatement(this, Operator::LIKE,
                              SqlStatement<T>(rt).getStatement());
    }

    // NOT LIKE
    template <StringLike T>
    SqlStatement<std::string> operator^(T&& rt) {
        return ::getStatement(this, Operator::NLIKE,
                              SqlStatement<T>(rt).getStatement());
    }

   protected:
    std::string name;
    PropertyType type;
    int id {-1};
};