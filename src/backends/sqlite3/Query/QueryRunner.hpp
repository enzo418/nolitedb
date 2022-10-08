#include <map>
#include <utility>

#include "nldb/DB/IDB.hpp"
#include "nldb/Property/ComposedProperty.hpp"
#include "nldb/Query/QueryRunner.hpp"

namespace nldb {
    class QueryRunnerSQ3 : public QueryRunner {
       public:
        QueryRunnerSQ3(IDB* connection);

       public:
        json select(QueryPlannerContextSelect&& data) override;
    };

    struct QueryRunnerCtx {
       public:
        /**
         * @brief Construct a new Query runner ctx
         *
         * @param rootCollectionID when you do from("users").select(...),
         * 'users' is the root collection. This filed is used to get the root id
         * value.
         */
        QueryRunnerCtx(int rootCollectionID);

       public:
        std::string_view getAlias(const Property& prop);

        std::string_view getAlias(const AggregatedProperty& agProp);

        std::string getValueExpression(const Property& prop);

        void set(const ComposedProperty& composed);

        int getRootCollId();

       private:
        // {prop_id, prop_coll_id} -> alias
        // this pair ^ is needed since properties of type ID doesn't have and id
        // because they are not really stored as properties in the database.
        std::map<std::pair<int, int>, std::string> props_aliases;

        // {prop_id, prop_coll_id} -> alias.value
        std::map<std::pair<int, int>, std::string> props_value_aliases;
        std::map<int, std::string> colls_aliases;

       private:
        std::string generateAlias(const Property& prop);
        std::string generateAlias(const AggregatedProperty& agProp);
        std::string generateValueExpression(const Property& prop);

        int rootCollectionID;
    };
}  // namespace nldb