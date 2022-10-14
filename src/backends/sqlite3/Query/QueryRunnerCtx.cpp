#include "QueryRunnerCtx.hpp"

#include "magic_enum.hpp"
#include "nldb/LOG/log.hpp"
#include "nldb/Utils/ParamsBindHelpers.hpp"

namespace nldb {

    QueryRunnerCtx::QueryRunnerCtx(int rootCollectionID, int pRootPropID,
                                   const std::string& pDocAlias)
        : rootCollectionID(rootCollectionID),
          rootPropId(pRootPropID),
          docAlias(pDocAlias) {};

    std::string QueryRunnerCtx::generateAlias(const Property& prop) {
        return utils::paramsbind::encloseQuotesConst(
            prop.getName() + "_" +
            std::to_string(prop.getType() == PropertyType::ID
                               ? prop.getCollectionId()
                               : prop.getId()));
    }

    std::string QueryRunnerCtx::generateAlias(
        const AggregatedProperty& agProp) {
        return "ag_" + std::string(magic_enum::enum_name(agProp.type)) + "_" +
               std::string(getAlias(agProp.property));
    }

    std::string QueryRunnerCtx::generateValueExpression(const Property& prop) {
        if (prop.getType() == PropertyType::ID) {
            int parentColl = prop.getCollectionId();

            if (parentColl == rootCollectionID) {
                return std::string(docAlias) + ".id";
            } else if (this->colls_aliases.contains(parentColl)) {
                return this->colls_aliases[parentColl] + ".id";
            } else {
                NLDB_ERROR("forgot to set the composed property with id {}",
                           parentColl);
            }
        }

        return std::string(getAlias(prop)) + ".value";
    }

    std::string_view QueryRunnerCtx::getAlias(const Property& prop) {
        int id = prop.getId();
        int collID = prop.getCollectionId();

        if (!props_aliases.contains({id, collID})) {
            props_aliases[{id, collID}] = this->generateAlias(prop);
        }

        return props_aliases.at({id, collID});
    }

    std::string_view QueryRunnerCtx::getAlias(
        const AggregatedProperty& agProp) {
        int id = agProp.property.getId();
        int collID = agProp.property.getCollectionId();

        if (!props_aliases.contains({id, collID})) {
            props_aliases[{id, collID}] = this->generateAlias(agProp.property);
        }

        return props_aliases.at({id, collID});
    }

    std::string QueryRunnerCtx::getValueExpression(const Property& prop) {
        int id = prop.getId();
        int collID = prop.getCollectionId();

        if (!props_value_aliases.contains({id, collID})) {
            props_value_aliases[{id, collID}] =
                this->generateValueExpression(prop);
        }

        return props_value_aliases[{id, collID}];
    }

    void QueryRunnerCtx::set(Object& composed) {
        auto& ref = composed.getPropertyRef();
        auto alias = this->getAlias(ref);

        colls_aliases[composed.getCollId()] = alias;
    }

    int QueryRunnerCtx::getRootCollId() { return rootCollectionID; }

    int QueryRunnerCtx::getRootPropId() { return rootPropId; }

}  // namespace nldb