#pragma once

#include <stdexcept>

#include "nldb/CommonConcepts.hpp"
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

    /**
     * @brief Reads a property from an expression.
     * Example:
     *      - population.current_year
     *         with collID = 1, which is the country collection, represents this
     *         year's population of a country
     *
     * @param expr
     * @param collID
     * @param repos
     * @return Property
     */
    Property readProperty(const std::string& expr, int collID,
                          Repositories* repos);

    namespace {
        /**
         * this structures will be the placeholder for the actual
         * properties/composed property while we parse the text.
         * Its better to do it this way because is easier to test.
         * The P_ stands for parser.
         */

        struct P_Prop {
            std::string name;
        };

        typedef std::variant<class P_Composed, P_Prop> P_SubProperty;

        struct P_Composed {
            P_Prop prop;
            std::vector<P_SubProperty> subProps;
        };

        /**
         * @brief Recursively reads all the properties from a string.
         * If IsFirst is true, it will return a ComposedProperty.
         *
         * @tparam IsFirst
         * @param c iterable char
         * @param end end of the string
         * @return P_SubProperty
         */
        template <bool IsFirst = true>
        P_SubProperty readNextPropertyRecursive(auto& c, const auto& end) {
            std::string word;

            while (c != end) {
                if (*c == '{') {
                    ++c;  // skip {
                    auto composed = P_Composed {word, {}};

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

                        composed.subProps.push_back(
                            readNextPropertyRecursive<false>(c, end));
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
                    return P_Prop {word};
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
                    return P_Composed {word};
                }

                throw std::runtime_error(
                    "Unexpected end of string while reading a property");
            } else {
                return P_Prop {word};
            }
        }
    }  // namespace

    namespace {
        template <typename IT>
        requires std::input_iterator<IT>  //
            Property covertInner(IT& it, int collID, Repositories* repos) {
            std::optional<Property> found =
                repos->repositoryProperty->find(collID, it->name);
            if (found) {
                if (found->getType() == PropertyType::OBJECT) {
                    auto subCollId =
                        repos->valuesDAO->findSubCollectionOfObjectProperty(
                            found->getId());
                    if (subCollId) {
                        return covertInner(++it, subCollId.value(), repos);
                    } else {
                        throw CollectionNotFound(it->name);
                    }
                } else {
                    return found.value();
                }
            } else {
                // instead of throwing, we could use a Property::zombie or empty
                throw PropertyNotFound(it->name);
            }
        }

        inline Property covertInner(std::vector<P_Prop>&& props, int collID,
                                    Repositories* repos) {
            auto it = props.begin();
            return covertInner(it, collID, repos);
        }
    }  // namespace
}  // namespace nldb::utils