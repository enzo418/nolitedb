#pragma once
#include <string>
#include <variant>

namespace nldb {
    enum PropertyType { ID, STRING, DOUBLE, INTEGER, OBJECT };

    class AggregatedProperty;
    class SortedProperty;
    class PropertyExpression;

    typedef std::variant<class Property, std::string, int, double, const char*>
        LogicConstValue;

    class Property {
       public:  // constructors
        Property(int id, const std::string& name, PropertyType type,
                 int collID);

       public:  // getters
        std::string getName() const;
        PropertyType getType() const;
        int getId() const;
        int getCollectionId() const;

       public:  // aggregate functions
        AggregatedProperty countAs(const char* alias);
        AggregatedProperty maxAs(const char* alias);
        AggregatedProperty minAs(const char* alias);
        AggregatedProperty sumAs(const char* alias);
        AggregatedProperty averageAs(const char* alias);

       public:  // sort functions
        SortedProperty desc();
        SortedProperty asc();

       public:  // logical r values
        PropertyExpression operator>(const LogicConstValue& right);
        PropertyExpression operator>=(const LogicConstValue& right);
        PropertyExpression operator<(const LogicConstValue& right);
        PropertyExpression operator<=(const LogicConstValue& right);
        PropertyExpression operator==(const LogicConstValue& right);
        PropertyExpression operator!=(
            const LogicConstValue& right);  // not equal
        PropertyExpression operator%(const LogicConstValue& right);  // like
        PropertyExpression operator^(const LogicConstValue& right);  // not like

       private:
        std::string name;
        PropertyType type;
        int id;
        int collectionId;
    };
}  // namespace nldb