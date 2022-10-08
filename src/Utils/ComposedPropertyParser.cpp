#include "nldb/Utils/ComposedPropertyParser.hpp"

#include "nldb/DAL/Repositories.hpp"
#include "nldb/Utils/Variant.hpp"

namespace nldb::utils {

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
            repos->valuesDAO->findSubCollectionOfObjectProperty(root->getId());

        if (!subCollId.has_value()) {
            return ComposedProperty::empty();
            // throw std::runtime_error("Couldn't find the sub-collection");
        }

        auto composed =
            ComposedProperty(root.value(), collID, subCollId.value(), {});

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

    ComposedProperty readComposedProperty(const std::string& str, int collID,
                                          Repositories* repos) {
        auto l_it = str.begin();  // create a l value iterator
        // we are sure it's a ComposedProperty since is the first iteration.
        return convertInner(
            std::get<P_Composed>(readNextPropertyRecursive(l_it, str.end())),
            collID, repos);
    }
}  // namespace nldb::utils