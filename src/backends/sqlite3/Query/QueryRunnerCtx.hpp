#pragma once

#include <string>

#include "nldb/DAL/Repositories.hpp"
#include "nldb/DB/IDB.hpp"
#include "nldb/Object.hpp"
#include "nldb/Query/QueryRunner.hpp"

namespace nldb {

    struct QueryRunnerCtx {
       public:
        /**
         * @brief Construct a new Query runner ctx
         *
         * @param rootCollectionID when you do from("users").select(...),
         * 'users' is the root collection. This filed is used to get the root id
         * value.
         * @param rootPropID is the owner of collection with id =
         * `rootCollectionID`
         */
        QueryRunnerCtx(snowflake rootCollectionID, snowflake rootPropID,
                       const std::string& doc_alias);

       public:
        std::string_view getAlias(const Property& prop);

        std::string_view getAlias(const AggregatedProperty& agProp);

        std::string getValueExpression(const Property& prop);

        void set(Object& composed);

        snowflake getRootCollId();
        snowflake getRootPropId();

       private:
        // {prop_id, prop_coll_id} -> alias
        // this pair ^ is needed since properties of type ID doesn't have and id
        // because they are not really stored as properties in the database.
        std::map<std::pair<snowflake, snowflake>, std::string> props_aliases;

        // {prop_id, prop_coll_id} -> alias.value
        std::map<std::pair<snowflake, snowflake>, std::string>
            props_value_aliases;
        std::map<snowflake, std::string> colls_aliases;

       private:
        std::string generateAlias(const Property& prop);
        std::string generateAlias(const AggregatedProperty& agProp);
        std::string generateValueExpression(const Property& prop);

        snowflake rootCollectionID;
        snowflake rootPropId;
        std::string docAlias;
    };
}  // namespace nldb