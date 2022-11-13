#pragma once
#include <optional>
#include <string>
#include <variant>

#include "nldb/typedef.hpp"

namespace nldb {
    enum PropertyType {
        ID,
        STRING,
        DOUBLE,
        INTEGER,
        OBJECT,
        ARRAY,
        _NULL,
        BOOLEAN
    };

    struct AggregatedProperty;
    struct SortedProperty;
    struct PropertyExpression;

    typedef std::variant<class Property, std::string, int, double, const char*>
        LogicConstValue;

    class Property {
       public:
        /**
         * @brief Construct a new property.
         * Represents the root property of a root collection.
         * You can't use this for subcollections as they don't have a
         * human-readable name.
         *
         * @param coll_name
         */
        Property(const std::string& coll_name);

        /**
         * @brief Constructor to use while building the query
         *
         * @param name
         * @param collName parent collection name
         */
        Property(const std::string& name, std::optional<std::string> collName);

        // to use while running the query
        Property(snowflake id, const std::string& name, PropertyType type,
                 snowflake collID);

       public:  // getters
        std::string getName() const;
        PropertyType getType() const;
        snowflake getId() const;
        snowflake getCollectionId() const;

       public:
        /**
         * @brief Returns false iff the parent collection name is not an
         * expression or it has no parent collection name.
         *
         * @return true
         * @return false
         */
        bool isParentNameAnExpression();

        /**
         * @brief Get the name of the parent collection.
         * It can be an identifier or an expression, check if before with
         * `isParentNameAnExpression()`.
         *
         * expression: <coll_name1>.<coll_name2>.<coll_name3>
         * expression example: users.phone
         *
         * @return std::string&
         */
        std::optional<std::string> getParentCollName();

       public:
        Property operator[](const std::string& name);

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
        std::optional<std::string> collName;

        std::string name;
        PropertyType type;
        snowflake id;
        snowflake collectionId;
    };
}  // namespace nldb
