#include "QueryRunnerCtx.hpp"

#include <algorithm>

#include "magic_enum.hpp"
#include "nldb/LOG/log.hpp"
#include "nldb/Property/Property.hpp"
#include "nldb/Utils/ParamsBindHelpers.hpp"
#include "nldb/typedef.hpp"

namespace nldb {

    QueryRunnerCtx::QueryRunnerCtx(snowflake rootCollectionID,
                                   snowflake pRootPropID,
                                   const std::string& pDocAlias)
        : rootCollectionID(rootCollectionID),
          rootPropId(pRootPropID),
          docAlias(pDocAlias) {};

    std::string QueryRunnerCtx::generateAlias(const Property& prop) {
#ifdef NLDB_DEBUG_QUERY
        // `NLDB_DEBUG_QUERY` is vulnerable to sql injection
        const std::string prefix = prop.getName();
#else
        const std::string prefix = "";
#endif

        auto type = prop.getType();

        if (type == PropertyType::ID) {
            return prefix + "___" + std::to_string(prop.getCollectionId());
        } else if (type == PropertyType::OBJECT) {
            // avoid collisions with property id in case we are not using
            // snowflake
            return prefix + "__" + std::to_string(prop.getId());
        } else {
            return prefix + "_" + std::to_string(prop.getId());
        }
    }

    std::string QueryRunnerCtx::getContextualizedAlias(
        const Property& prop, snowflake statementCollId) {
        bool isPassThrough = prop.getCollectionId() == statementCollId;
        return isPassThrough ? getValueExpression(prop)
                             : std::string(getAlias(prop));
    }

    std::string QueryRunnerCtx::generateAlias(
        const AggregatedProperty& agProp) {
        return "ag_" + std::string(magic_enum::enum_name(agProp.type)) + "_" +
               std::string(getAlias(agProp.property));
    }

    std::string QueryRunnerCtx::generateValueExpression(const Property& prop) {
        snowflake parentColl = prop.getCollectionId();

        if (prop.getType() == PropertyType::ID) {
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
        snowflake id = prop.getId();
        snowflake collID = prop.getCollectionId();

        if (!props_aliases.contains({id, collID})) {
            props_aliases[{id, collID}] = this->generateAlias(prop);
        }

        return props_aliases.at({id, collID});
    }

    std::string_view QueryRunnerCtx::getAlias(
        const AggregatedProperty& agProp) {
        snowflake id = agProp.property.getId();
        snowflake collID = agProp.property.getCollectionId();

        if (!props_aliases.contains({id, collID})) {
            props_aliases[{id, collID}] = this->generateAlias(agProp.property);
        }

        return props_aliases.at({id, collID});
    }

    std::string QueryRunnerCtx::getValueExpression(const Property& prop) {
        snowflake id = prop.getId();
        snowflake collID = prop.getCollectionId();

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

    snowflake QueryRunnerCtx::getRootCollId() { return rootCollectionID; }

    snowflake QueryRunnerCtx::getRootPropId() { return rootPropId; }

}  // namespace nldb