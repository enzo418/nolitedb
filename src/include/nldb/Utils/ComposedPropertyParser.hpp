#pragma once

#include <stdexcept>

#include "nldb/DAL/Repositories.hpp"
#include "nldb/Exceptions.hpp"
#include "nldb/Property/ComposedProperty.hpp"
#include "nldb/Property/Property.hpp"

namespace nldb::utils {
    /**
     * @brief Converts an expression to a composed property. See
     * ComposedProperty evaluate function.
     *
     * @param expr expression
     * @param collID parent collection id
     * @param repos repositories from which to obtain data
     * @return ComposedProperty
     */
    ComposedProperty readComposedProperty(const std::string& expr, int collID,
                                          Repositories* repos);

    namespace {
        inline ComposedProperty getComposed(const std::string& name, int collID,
                                            Repositories* repos) {
            auto objProp = repos->repositoryProperty->find(collID, name);

            if (!objProp.has_value()) {
                return ComposedProperty::empty();
                // throw PropertyNotFound(name);
            }

            auto subCollId =
                repos->valuesDAO->findSubCollectionOfObjectProperty(
                    objProp->getId());

            if (!subCollId.has_value()) {
                throw std::runtime_error("Couldn't find the sub-collection");
            }

            return ComposedProperty(objProp.value(), collID, subCollId.value(),
                                    {});
        }

        /**
         * @brief Recursively reads all the properties from a string.
         * If IsFirst is true, it will return a ComposedProperty.
         *
         * @tparam IsFirst
         * @param c iterable char
         * @param end end of the string
         * @param collID curent parent collection id
         * @param repos repositories
         * @return SubProperty
         */
        template <bool IsFirst = true>
        SubProperty readNextPropertyRecursive(auto& c, const auto& end,
                                              int collID, Repositories* repos) {
            std::string word;

            while (c != end) {
                if (*c == '{') {
                    ++c;  // skip {
                    auto composed = getComposed(word, collID, repos);

                    if (composed.isEmpty()) return composed;

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

                        composed.addProperty(readNextPropertyRecursive<false>(
                            c, end, composed.getSubCollectionId(), repos));
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
                    auto prop = repos->repositoryProperty->find(collID, word);
                    if (!prop) throw PropertyNotFound(word);
                    return prop.value();
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
                    return getComposed(word, collID, repos);
                }

                throw std::runtime_error(
                    "Unexpected end of string while reading a property");
            } else {
                auto prop = repos->repositoryProperty->find(collID, word);
                if (!prop) throw PropertyNotFound(word);
                return prop.value();
            }
        }
    }  // namespace
}  // namespace nldb::utils