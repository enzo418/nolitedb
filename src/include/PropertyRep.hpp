#pragma once
#include <functional>
#include <optional>
#include <string>
#include <type_traits>

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

enum AGGREGATEFUNCTIONTYPE { COUNT, AVG, SUM, MAX, MIN };

/**
 * IDEA
 #include <iostream>
#include <set>
#include <string>

struct LoF;

enum TP {R = 23};
enum HTP {E = 99};

struct IProp {
    IProp(TP type) : type(type) {};
    TP type;
};


struct HoF : public IProp {
    HoF(LoF* l) :  IProp(R), l(l) {}

    HTP e{E};
    LoF* l;
};

struct LoF : public IProp {
    LoF(TP type) : IProp(type) {};

    TP getType() { return IProp::type; };

    HoF getH() { return HoF(this); }
};

void f(const IProp& l) {
    if (l.type == R) {
        auto c = (const HoF&) l;
        std::cout << c.e << std::endl;
        std::cout << c.l->getType() << std::endl;
    }
}

int main()
{
    LoF l(R);
    //HoF h(l);
    f(l.getH());

}

 *
 */

struct AggregateFunction {
    AggregateFunction(PropertyRep* pProp, const char* pAlias,
                      AGGREGATEFUNCTIONTYPE pType)
        : prop(pProp), alias(pAlias), type(pType) {}
    const char* alias;
    AGGREGATEFUNCTIONTYPE type;
    PropertyRep* prop;
};

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

   public:  // aggregate functions
    AggregateFunction count(const char* alias);
    AggregateFunction average(const char* alias);
    AggregateFunction min(const char* alias);
    AggregateFunction max(const char* alias);
    AggregateFunction sum(const char* alias);

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
