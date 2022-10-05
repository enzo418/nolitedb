#pragma once
#include "nldb/nldb_json.hpp"

namespace nldb {

    // IQueryRunner -> QueryContext -> queryRunner:IQueryRunner -> IQueryRunner
    // -> ...
    class QueryPlannerContextUpdate;
    class QueryPlannerContextInsert;
    class QueryPlannerContextRemove;
    class QueryPlannerContextSelect;

    class IQueryRunner {
       public:
        virtual json select(QueryPlannerContextSelect&& data) = 0;
        virtual void update(QueryPlannerContextUpdate&& data) = 0;
        virtual void insert(QueryPlannerContextInsert&& data) = 0;
        virtual void remove(QueryPlannerContextRemove&& data) = 0;
    };
}  // namespace nldb