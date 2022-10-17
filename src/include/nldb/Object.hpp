#pragma once

#include <string>
#include <vector>

#include "nldb/Property/Property.hpp"

namespace nldb {

    typedef std::variant<class Object, Property> SubProperty;

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
        Object(Property prop, std::vector<SubProperty>&& properties,
               int collId = -1);

        Object(Property prop, const std::vector<SubProperty>& properties,
               int collId = -1);

       public:
        Property getProperty() const;  // the property of type object
        Property& getPropertyRef();
        std::vector<SubProperty> getProperties() const;
        std::vector<SubProperty>& getPropertiesRef();

        int getCollId() const;

       public:
        void addProperty(SubProperty prop);
        void setCollId(int pCollId);

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
        std::vector<SubProperty> properties;
        int collID;  // the id of the collection it represents
    };
}  // namespace nldb