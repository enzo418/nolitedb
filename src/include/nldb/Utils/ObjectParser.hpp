#pragma once

#include <array>
#include <stdexcept>

#include "nldb/CommonConcepts.hpp"
#include "nldb/Exceptions.hpp"
#include "nldb/Object.hpp"
#include "nldb/Property/Property.hpp"

namespace nldb::utils {
    /**
     * @brief Converts an expression to a composed property. See
     * Object evaluate function.
     *
     * @param expr expression
     * @param collName parent collection name
     * @return Object
     */
    Object expandObjectExpression(const std::string& expr,
                                  const std::string& collName);

    /**
     * @brief Reads a property from an expression.
     * Example:
     *      - population.current_year
     *         with collName = "country", we get this year's population of a
     *         country
     *
     * @param expr
     * @param collName parent collection name
     * @return Property
     */
    // Property getPropertyFromExpression(const std::string& expr,
    //                                    const std::string& collName);

    /**
     * @brief Checks if a string follows the object syntax.
     * Constexpr function that does the same as readNextPropertyRecursive but is
     * used to avoid the _obj postfix.
     *
     * @tparam IsFirst is first call to this function?
     * @tparam N char array length
     * @param start from where to start the test
     * @return constexpr std::array<size_t, 2> first element is a boolean wether
     * the read string was valid or not, last element is the last position
     * checked
     */
    template <bool IsFirst, size_t N>
    constexpr std::array<size_t, 2> isValidObject(const char (&exp)[N],
                                                  size_t start = 0) {
        size_t i = start;
        char c = exp[i];

        while (i < N) {
            c = exp[i];

            if (c == '{') {
                ++i;  // skip {

                if (exp[i] == ',' || exp[i] == '}') {
                    return {false, i};
                }

                do {
                    if (exp[i] == ',') i++;    // skip , to read the prop name
                    while (exp[i] <= 32) i++;  // skip spaces

                    auto result = isValidObject<false>(exp, i);
                    if (!result[0]) {
                        return {false, i};
                    } else {
                        i = result[1];
                    }

                } while (exp[i] == ',');

                if (exp[i] == '}') {  // composed ended
                    i++;
                    return {true, i};
                } else {
                    return {false, i};
                }
            } else if (c == '}' || c == ',') {
                if constexpr (IsFirst) {
                    // Unexpected character at the root property
                    return {false, i};
                }

                // found a } or a , outside a composed, that means that
                // we are just a property.
                return {true, i};
            } else if (c > 32) {
                i++;
            } else {
                return {false, i};
            }
        }

        if constexpr (IsFirst) {
            return {i > 0, i};
        } else {
            return {true, i};
        }
    }

    namespace {
        /**
         * @brief Recursively reads all the properties from a string.
         * If IsFirst is true, it will return a Object.
         *
         * @tparam IsFirst
         * @param c iterable char
         * @param end end of the string
         * @param collName parent collection name
         * @return SubProperty
         */
        template <bool IsFirst = true>
        SubProperty readNextPropertyRecursive(auto& c, const auto& end,
                                              const std::string& collName) {
            std::string word;

            while (c != end) {
                if (*c == '{') {
                    ++c;  // skip {
                    auto composed = Object {Property(word, collName), {}};

                    // read its properties

                    if (*c == ',' || *c == '}') {
                        // errors could easily be improved
                        throw std::runtime_error(
                            "expected property name at -> " +
                            std::string(c, end));
                    }

                    do {
                        if (*c == ',') c++;    // skip , to read the prop name
                        while (*c <= 32) c++;  // skip spaces

                        // if we dont keep the property context (parents tree)
                        // then we can't find its parent collection.
                        const auto parentExpr = collName.length() > 0
                                                    ? collName + "." + word
                                                    : word;

                        composed.addProperty(readNextPropertyRecursive<false>(
                            c, end, parentExpr));
                    } while (*c == ',');

                    if (*c == '}') {  // composed ended
                        c++;
                        return composed;
                    } else {
                        throw std::runtime_error(
                            "Syntax error on composed property, expected '}' "
                            "at the end of the collection properties " +
                            word);
                    }
                } else if (*c == '}' || *c == ',') {
                    if constexpr (IsFirst) {
                        throw std::runtime_error("Unexpected character '" +
                                                 std::string(*c, 1) +
                                                 "' at the root property");
                    }

                    // found a } or a , outside a composed, that means that
                    // we are just a property.
                    return Property {word, collName};
                } else if (*c > 32) {
                    word += *c++;
                } else {
                    throw std::runtime_error(
                        "Syntax error on composed property near " +
                        std::string(c, end));
                }
            }

            if constexpr (IsFirst) {
                if (word.length() != 0) {
                    return Object {Property(word, collName), {}};
                }

                throw std::runtime_error(
                    "Unexpected end of string while reading a property");
            } else {
                return Property {word, collName};
            }
        }
    }  // namespace
}  // namespace nldb::utils