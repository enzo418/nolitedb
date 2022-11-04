#include "BufferDataSQ3.hpp"

#include <map>
#include <sstream>

#include "backends/sqlite3/DAL/Definitions.hpp"
#include "nldb/LOG/log.hpp"
#include "nldb/Profiling/Profiler.hpp"
#include "nldb/Property/Property.hpp"
#include "nldb/Utils/ParamsBindHelpers.hpp"

namespace nldb {
    using namespace definitions;

    BufferDataSQ3::BufferDataSQ3(IDB* db) : BufferData(db) {}

    void BufferDataSQ3::pushRootProperties() {
        NLDB_PROFILE_FUNCTION();

        std::stringstream root_prop_sql;
        root_prop_sql << "insert into property (id, name, type) values ";

        bufferRootProperty.ForEach(
            [&root_prop_sql](BufferValueRootProperty& val, bool isLast) {
                root_prop_sql << "(" << val.id << ", "
                              << utils::paramsbind::encloseQuotesConst(val.name)
                              << ", " << (int)PropertyType::OBJECT << ")"
                              << (isLast ? ";" : ",");
            });

        const std::string root_prop_str = std::move(root_prop_sql).str();

        conn->execute(root_prop_str, {});

        bufferRootProperty.Reset();
    }

    void BufferDataSQ3::pushCollections() {
        NLDB_PROFILE_FUNCTION();

        std::stringstream coll_sql;
        coll_sql << "insert into collection (id, name, owner_id) values ";

        bufferCollection.ForEach(
            [&coll_sql](BufferValueCollection& val, bool isLast) {
                coll_sql << "(" << val.id << ", "
                         << utils::paramsbind::encloseQuotesConst(val.name)
                         << ", " << val.owner_id << ")" << (isLast ? ";" : ",");
            });

        const std::string coll_sql_str = std::move(coll_sql).str();

        conn->execute(coll_sql_str, {});

        bufferCollection.Reset();
    }

    void BufferDataSQ3::pushProperties() {
        NLDB_PROFILE_FUNCTION();

        std::stringstream prop_sql;
        prop_sql << "insert into property (id, name, type, coll_id) values ";

        bufferProperty.ForEach(
            [&prop_sql](BufferValueProperty& val, bool isLast) {
                prop_sql << "(" << val.id << ", "
                         << utils::paramsbind::encloseQuotesConst(val.name)
                         << ", " << (int)val.type << "," << val.coll_id << ")"
                         << (isLast ? ";" : ",");
            });

        const std::string prop_str = std::move(prop_sql).str();

        conn->execute(prop_str, {});

        bufferProperty.Reset();
    }

    void BufferDataSQ3::pushIndependentObjects() {
        NLDB_PROFILE_FUNCTION();

        auto& tables = tables::getPropertyTypesTable();

        // first insert the objects that does not depend on other
        // objects because values depends on them
        std::stringstream indep_sql;
        indep_sql << utils::paramsbind::parseSQL(
            "insert into @table (id, prop_id) values ",
            {{"@table", tables[PropertyType::OBJECT]}}, false);

        /**
         ** TODO: check if the query is longer that the DB supports
         **/

        bufferIndependentObject.ForEach(
            [&indep_sql](BufferValueIndependentObject& val, bool isLast) {
                indep_sql << "(" << val.id << ", " << val.prop_id << ")"
                          << (isLast ? ";" : ",");
            });

        const std::string indep_str = std::move(indep_sql).str();

        conn->execute(indep_str, {});

        bufferIndependentObject.Reset();
    }

    void BufferDataSQ3::pushDependentObjects() {
        NLDB_PROFILE_FUNCTION();

        auto& tables = tables::getPropertyTypesTable();

        // now that we have inserted the object that does not depend on other
        // objects, we can insert the objects that do depend on other objects.
        // Else the foreign key wouldn't exist.

        std::stringstream dependent_sql;
        dependent_sql << utils::paramsbind::parseSQL(
            "insert into @table (id, prop_id, obj_id) values ",
            {{"@table", tables[PropertyType::OBJECT]}}, false);

        /**
         ** TODO: check if the query is longer that the DB supports
         **/

        bufferDependentObject.ForEach(
            [&dependent_sql](BufferValueDependentObject& val, bool isLast) {
                dependent_sql << "(" << val.id << "," << val.prop_id << ","
                              << val.obj_id << ")" << (isLast ? ";" : ",");
            });

        const std::string dependent_s = std::move(dependent_sql).str();

        conn->execute(dependent_s, {});

        bufferDependentObject.Reset();
    }

    void BufferDataSQ3::pushStringLikeValues() {
        NLDB_PROFILE_FUNCTION();

        auto& tables = tables::getPropertyTypesTable();

        std::map<PropertyType, std::stringstream> queries;

        bufferStringLike.ForEach(
            [&queries, &tables](BufferValueStringLike& val, bool) {
                if (!queries.contains(val.type)) {
                    queries[val.type] = {};
                    queries[val.type] << utils::paramsbind::parseSQL(
                        "insert into @table (prop_id, obj_id, value) values ",
                        {{"@table", tables[val.type]}}, false);
                }

                /**
                 * TODO: check if the query is longer that the DB supports
                 */

                queries[val.type]
                    << "(" << val.propID << "," << val.objID << ","
                    << utils::paramsbind::encloseQuotesConst(val.value) << "),";
            });

        std::stringstream sql;

        for (auto& [p, subq] : queries) {
            sql << subq.rdbuf();
            // change last , for a ;
            sql.seekp(sql.tellp() - 1l);
            sql << ";";
        }

        const std::string depVal_s = std::move(sql).str();

        conn->execute(depVal_s, {});

        bufferStringLike.Reset();
    }

}  // namespace nldb