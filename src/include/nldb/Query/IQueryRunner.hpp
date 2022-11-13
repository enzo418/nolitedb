#pragma once
#include "nldb/nldb_json.hpp"

namespace nldb {

    // IQueryRunner -> QueryContext -> queryRunner:IQueryRunner -> IQueryRunner
    // -> ...
    struct QueryPlannerContextUpdate;
    struct QueryPlannerContextInsert;
    struct QueryPlannerContextRemove;
    struct QueryPlannerContextSelect;

    class IQueryRunner {
       public:
        virtual json select(QueryPlannerContextSelect&& data) = 0;
        virtual void update(QueryPlannerContextUpdate&& data) = 0;
        virtual void insert(QueryPlannerContextInsert&& data) = 0;
        virtual void remove(QueryPlannerContextRemove&& data) = 0;

        virtual ~IQueryRunner() = default;
    };
}  // namespace nldb