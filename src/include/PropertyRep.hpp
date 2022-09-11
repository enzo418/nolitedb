#pragma once
#include <functional>
#include <string>
#include <type_traits>

#include "CommonTypes.hpp"
#include "Concepts.hpp"
#include "Enums.hpp"
#include "PropertyCondition.hpp"

template <typename T>
concept StringLikeOrProperty = (StringLike<T> ||
                                std::is_same<PropertyRep, T>::value);

class PropertyRep {
   public:
    PropertyRep(const std::string& pName);

   public:
    PropertyCondition operator<(PropertyRep& rt) {
        return PropertyCondition(this, Condition::LT, &rt);
    }

    PropertyCondition operator<=(PropertyRep& rt) {
        return PropertyCondition(this, Condition::LTE, &rt);
    }

    PropertyCondition operator>(PropertyRep& rt) {
        return PropertyCondition(this, Condition::GT, &rt);
    }

    PropertyCondition operator>=(PropertyRep& rt) {
        return PropertyCondition(this, Condition::GTE, &rt);
    }

    // EQUAL
    PropertyCondition operator==(PropertyRep& rt) {
        return PropertyCondition(this, Condition::EQ, &rt);
    }

    // NOT EQUAL
    PropertyCondition operator!=(PropertyRep& rt) {
        return PropertyCondition(this, Condition::NEQ, &rt);
    }

    // LIKE
    PropertyCondition operator%(PropertyRep& rt) {
        return PropertyCondition(this, Condition::LIKE, &rt);
    }

    // NOT LIKE
    PropertyCondition operator^(PropertyRep& rt) {
        return PropertyCondition(this, Condition::NLIKE, &rt);
    }

   public:  // overloads
    template <typename T>
    PropertyCondition operator<(T&& rt) {
        return PropertyCondition(this, Condition::LT, rt);
    }

    template <typename T>
    PropertyCondition operator<=(T&& rt) {
        return PropertyCondition(this, Condition::LTE, rt);
    }

    template <typename T>
    PropertyCondition operator>(T&& rt) {
        return PropertyCondition(this, Condition::GT, rt);
    }

    template <typename T>
    PropertyCondition operator>=(T&& rt) {
        return PropertyCondition(this, Condition::GTE, rt);
    }

    // EQUAL
    template <typename T>
    PropertyCondition operator==(T&& rt) {
        return PropertyCondition(this, Condition::EQ, rt);
    }

    // NOT EQUAL
    template <typename T>
    PropertyCondition operator!=(T&& rt) {
        return PropertyCondition(this, Condition::NEQ, rt);
    }

    // LIKE
    template <StringLike T>
    PropertyCondition operator%(T&& rt) {
        return PropertyCondition(this, Condition::LIKE, rt);
    }

    // NOT LIKE
    template <StringLike T>
    PropertyCondition operator^(T&& rt) {
        return PropertyCondition(this, Condition::NLIKE, rt);
    }

   public:
    std::string_view getName();

   protected:
    std::string name;
};