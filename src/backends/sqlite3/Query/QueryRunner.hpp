#include "nldb/DB/IDB.hpp"
#include "nldb/Query/QueryRunner.hpp"

namespace nldb {
    class QueryRunnerSQ3 : public QueryRunner {
       public:
        QueryRunnerSQ3(IDB* connection);

       public:
        json select(QueryPlannerContextSelect&& data) override;
    };
}  // namespace nldb