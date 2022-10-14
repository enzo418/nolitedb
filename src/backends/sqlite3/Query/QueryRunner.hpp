#include <map>
#include <utility>

#include "QueryRunnerCtx.hpp"
#include "nldb/DAL/Repositories.hpp"
#include "nldb/DB/IDB.hpp"
#include "nldb/Object.hpp"
#include "nldb/Query/QueryRunner.hpp"

namespace nldb {
    class QueryRunnerSQ3 : public QueryRunner {
       public:
        QueryRunnerSQ3(IDB* connection, std::shared_ptr<Repositories> repos);

       public:
        json select(QueryPlannerContextSelect&& data) override;
    };
}  // namespace nldb