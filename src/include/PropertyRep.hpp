#pragma once
#include <functional>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>

#include "CommonTypes.hpp"
#include "Concepts.hpp"
#include "Enums.hpp"
#include "ISqlStatement.hpp"
#include "SqlExpression.hpp"
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

typedef std::variant<std::string, int, double, const char*> RightValue;

struct SortProp {
    SortProp(PropertyRep* pProp, SortType pType) : prop(pProp), type(pType) {}

    SortType type;
    PropertyRep* prop;
};

struct AggregateFunction {
    AggregateFunction(PropertyRep* pProp, const char* pAlias,
                      AggregateType pType)
        : prop(pProp), alias(pAlias), type(pType) {}
    const char* alias;
    AggregateType type;
    PropertyRep* prop;
};

namespace tables {
    std::unordered_map<PropertyType, std::string>& getPropertyTables();
};

class PropertyRep : public ISqlStatement {
   public:
    PropertyRep(const std::string& pName, int id, PropertyType type);

   public:
    /**
     * @brief Get the internal id of the property
     * @return int
     */
    int getId() const;

    /**
     * @brief Get its type
     *
     * @return PropertyType
     */
    PropertyType getType() const;

    /**
     * @brief Get the property name or alias
     *
     * @return std::string_view
     */
    std::string_view getName() const;
    std::string getStatement() const override;

    std::string getValueExpression() const;

    static std::string getTableNameForTypeValue(PropertyType type);

    static std::optional<PropertyRep> find(IDB* ctx, int collectionID,
                                           const std::string& name);

   public:  // aggregate functions
    AggregateFunction countAs(const char* alias);
    AggregateFunction averageAs(const char* alias);
    AggregateFunction minAs(const char* alias);
    AggregateFunction maxAs(const char* alias);
    AggregateFunction sumAs(const char* alias);

   public:  // sort order
    SortProp asc();
    SortProp desc();

   public:
    SqlLogicExpression operator<(PropertyRep& rt);

    SqlLogicExpression operator<=(PropertyRep& rt);

    SqlLogicExpression operator>(PropertyRep& rt);

    SqlLogicExpression operator>=(PropertyRep& rt);

    // EQUAL
    SqlLogicExpression operator==(PropertyRep& rt);

    // NOT EQUAL
    SqlLogicExpression operator!=(PropertyRep& rt);

    // LIKE
    SqlLogicExpression operator%(PropertyRep& rt);

    // NOT LIKE
    SqlLogicExpression operator^(PropertyRep& rt);

    // -- Same operator but for other types
    SqlLogicExpression operator<(RightValue rt);

    SqlLogicExpression operator<=(RightValue rt);

    SqlLogicExpression operator>(RightValue rt);

    SqlLogicExpression operator>=(RightValue rt);

    // EQUAL
    SqlLogicExpression operator==(RightValue rt);

    // NOT EQUAL
    SqlLogicExpression operator!=(RightValue rt);

    // LIKE
    SqlLogicExpression operator%(RightValue rt);

    // NOT LIKE
    SqlLogicExpression operator^(RightValue rt);

   private:
    std::string generateConditionStatement(Operator, PropertyRep& rt);
    std::string generateConditionStatement(Operator, RightValue rt);

   protected:
    std::string name;
    PropertyType type;
    int id {-1};
};
