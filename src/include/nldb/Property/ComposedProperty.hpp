#pragma once

#include <string>
#include <vector>

#include "Property.hpp"
#include "nldb/DAL/Repositories.hpp"

namespace nldb {

    typedef std::variant<class ComposedProperty, Property> SubProperty;

    class ComposedPropertyExpression {
       public:
        constexpr ComposedPropertyExpression(const char* n, std::size_t size)
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
    class ComposedProperty {
       public:
        /**
         * @brief Construct a new Composed Property object
         *
         * e.g. if user is {name, contact{email, phone, smoke_signal}}, and
         * Composed Property is contact{email, phone} then
         *
         * @param prop is the property 'contact' object from the collection
         * 'user'
         * @param subCollID 'contact' collection id
         * @param properties 'email' and 'phone'
         * @param repos repositories, needed by operator[]
         */
        ComposedProperty(Property prop, int subCollID,
                         std::vector<SubProperty>&& properties,
                         Repositories* repos);

        ComposedProperty(Property prop, int subCollID,
                         const std::vector<SubProperty>& properties,
                         Repositories* repos);

       public:
        Property getProperty() const;  // the property of type object
        int getSubCollectionId() const;
        std::vector<SubProperty> getProperties() const;
        std::vector<SubProperty>& getPropertiesRef();

       public:
        void addProperty(SubProperty prop);

       public:
        bool isEmpty();
        static ComposedProperty empty();

       public:
        /**
         * @brief Creates a composed property from an expression string.
         * Filter object properties using
         * {prop1, prop2{prop21, prop22},...}
         *
         * @param repos
         * @param expr
         * @return ComposedProperty
         */
        static ComposedProperty evaluateExpresion(
            int collID, Repositories* repos,
            const ComposedPropertyExpression& expr);

       public:
        /**
         * @brief Evaluates an expression an return the corresponding property.
         *
         * @param expr
         * @return Property
         */
        Property operator[](const char* expr);

       private:
        Property prop;
        int subCollectionID;  // id of the sub-collection that it holds
        std::vector<SubProperty> properties;
        bool flag_empty {false};
        Repositories* repos;
    };
}  // namespace nldb