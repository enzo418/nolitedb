#pragma once

#include <forward_list>
#include <string>
#include <vector>

#include "nldb/Property/AggregatedProperty.hpp"
#include "nldb/Property/Property.hpp"
#include "nldb/typedef.hpp"

namespace nldb {

    typedef std::variant<class Object, Property, AggregatedProperty>
        SubProperty;

    class ObjectExpression {
       public:
        constexpr ObjectExpression(const char* n, std::size_t size)
            : expr(n), size(size) {};

       public:
        const char* expr;
        std::size_t size;
    };

    /**
     * @brief A property of type object with properties (an object).
     * Example: if user is {name, contact{email, phone}}, a Composed
     * Property could be as contact{email}.
     */
    class Object {
       public:
        /**
         * @brief Construct a new Object
         *
         * e.g. if user is {name, contact{email, phone, smoke_signal}}, and
         * Object is "contact{email, phone}" then
         *
         * @param prop is the property 'contact' object from the collection
         * 'user'
         * @param properties 'email' and 'phone'
         * @param collId the id of the collection it represents, `contact`.id
         */
        Object(Property prop, std::forward_list<SubProperty>&& properties,
               snowflake collId = -1);

        Object(Property prop, const std::forward_list<SubProperty>& properties,
               snowflake collId = -1);

       public:
        Property getProperty() const;  // the property of type object
        Property& getPropertyRef();
        std::forward_list<SubProperty> getProperties() const;
        std::forward_list<SubProperty>& getPropertiesRef();

        snowflake getCollId() const;

       public:
        SubProperty& addProperty(SubProperty prop);
        void setCollId(snowflake pCollId);

       public:
        /**
         * @brief Creates a composed property from an expression string.
         * Filter object properties using
         * {prop1, prop2{prop21, prop22},...}
         *
         * @param expr
         * @param collName parent collection name
         * @return ComposedProperty
         */
        static Object evaluateExpresion(const ObjectExpression& expr,
                                        const std::string& collName);

       public:
        /**
         * @brief Evaluates an expression an return the corresponding property.
         *
         * @param expr
         * @return Property
         */
        Property operator[](const std::string& expr);

       private:
        Property prop;
        std::forward_list<SubProperty> properties;
        snowflake collID;  // the id of the collection it represents
    };
}  // namespace nldb