#include "nldb/Utils/ComposedPropertyParser.hpp"

#include "nldb/DAL/Repositories.hpp"
#include "nldb/Utils/Variant.hpp"

namespace nldb::utils {

    namespace fieldsFilter {

        Property convertInner(const P_Prop& p_prop, int collID,
                              Repositories* repos) {
            auto prop = repos->repositoryProperty->find(collID, p_prop.name);
            if (!prop) throw PropertyNotFound();
            return prop.value();
        }

        /**
         * @param composed
         * @param collID  collID curent parent collection id
         * @param repos  repositories
         * @return ComposedProperty
         */
        ComposedProperty convertInner(const P_Composed& p_composed, int collID,
                                      Repositories* repos) {
            auto root =
                repos->repositoryProperty->find(collID, p_composed.prop.name);

            if (!root) return ComposedProperty::empty();

            auto subCollId =
                repos->valuesDAO->findSubCollectionOfObjectProperty(
                    root->getId());

            if (!subCollId.has_value()) {
                return ComposedProperty::empty();
                // throw std::runtime_error("Couldn't find the sub-collection");
            }

            auto composed =
                ComposedProperty(root.value(), subCollId.value(), {}, repos);

            auto cb = overloaded {
                [&repos, &composed](const P_Prop& p) {
                    composed.addProperty(
                        convertInner(p, composed.getSubCollectionId(), repos));
                },
                [&repos, &composed](const P_Composed& p) {
                    auto res =
                        convertInner(p, composed.getSubCollectionId(), repos);
                    if (!res.isEmpty()) {
                        composed.addProperty(res);
                    }
                }
                //
            };

            for (auto& prop : p_composed.subProps) {
                std::visit(cb, prop);
            }

            return composed;
        }
    }  // namespace fieldsFilter

    namespace dotProperties {
        std::vector<P_Prop> readDotSeparatedProperties(
            const std::string& expr) {
            std::vector<P_Prop> props;
            unsigned long last_pos = 0;

            while (true) {
                int pos = expr.find('.', last_pos);

                if (pos != expr.npos) {
                    props.push_back(
                        P_Prop {expr.substr(last_pos, pos - last_pos)});
                    last_pos = pos + 1;
                } else {
                    if (last_pos < expr.length() - 1) {
                        props.push_back(P_Prop {
                            expr.substr(last_pos, last_pos - expr.length())});
                    }

                    break;
                }
            }

            return std::move(props);
        }

    }  // namespace dotProperties

    ComposedProperty readComposedProperty(const std::string& str, int collID,
                                          Repositories* repos) {
        auto l_it = str.begin();  // create a l value iterator
        // we are sure it's a ComposedProperty since is the first iteration.
        return fieldsFilter::convertInner(
            std::get<P_Composed>(readNextPropertyRecursive(l_it, str.end())),
            collID, repos);
    }

    Property readProperty(const std::string& expr, int collID,
                          Repositories* repos) {
        return covertInner(dotProperties::readDotSeparatedProperties(expr),
                           collID, repos);
    }
}  // namespace nldb::utils