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

SqlStatement<std::string> getStatement(PropertyRep* lf, Operator op,
                                       const char* rt);

class PropertyRep;

typedef std::variant<PropertyRep, std::string, int, double, const char*>
    RightValue;

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
    SqlStatement<std::string> operator<(RightValue rt);

    SqlStatement<std::string> operator<=(RightValue rt);

    SqlStatement<std::string> operator>(RightValue rt);

    SqlStatement<std::string> operator>=(RightValue rt);

    // EQUAL
    SqlStatement<std::string> operator==(RightValue rt);

    // NOT EQUAL
    SqlStatement<std::string> operator!=(RightValue rt);

    // LIKE
    SqlStatement<std::string> operator%(RightValue rt);

    // NOT LIKE
    SqlStatement<std::string> operator^(RightValue rt);

   protected:
    std::string name;
    PropertyType type;
    int id {-1};
};