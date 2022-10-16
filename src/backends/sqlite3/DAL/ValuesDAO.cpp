#include "ValuesDAO.hpp"

#include <map>
#include <optional>
#include <sstream>
#include <unordered_map>

#include "backends/sqlite3/DAL/Definitions.hpp"
#include "magic_enum.hpp"
#include "nldb/LOG/log.hpp"
#include "nldb/Property/Property.hpp"
#include "nldb/Utils/ParamsBindHelpers.hpp"

namespace nldb {
    using namespace definitions;

    constexpr int max_sql_query_length = 1000000000;

    // how many as you can fit in 0.333 MB
    template <typename T>
    constexpr int bufferSize() {
        return (int)(1e6 / 3 / sizeof(T));
    };

    ValuesDAO::ValuesDAO(IDB* pConnection)
        : conn(pConnection),
          bufferStringLike(bufferSize<BufferValueStringLike>()),
          bufferDependentObject(bufferSize<BufferValueDependentObject>()),
          bufferIndependentObject(bufferSize<BufferValueIndependentObject>()) {}

    void ValuesDAO::addStringLike(int propID, snowflake objID,
                                  PropertyType type, std::string value) {
        if (bufferDependentObject.Size() + bufferIndependentObject.Size() > 0) {
            NLDB_WARN(
                "OBJECTS WERE STILL NOT ADDED, THIS INSERTION MIGHT FAIL.");
        }

        const std::string sql =
            "insert into @table (obj_id, prop_id, value) values (@obj_id, "
            "@prop_id, @value);";

        conn->execute(sql, {{"@table", tables::getPropertyTypesTable()[type]},
                            {"@obj_id", objID},
                            {"@prop_id", propID},
                            {"@value", value}});
    }

    snowflake ValuesDAO::addObject(int propID) {
        if (bufferDependentObject.Size() + bufferIndependentObject.Size() > 0) {
            NLDB_WARN(
                "OBJECTS WERE STILL NOT ADDED, THIS INSERTION MIGHT FAIL.");
        }

        const std::string sql =
            "insert into @table (prop_id) "
            "values (@prop_id);";

        conn->execute(
            sql,
            {{"@table", tables::getPropertyTypesTable()[PropertyType::OBJECT]},
             {"@prop_id", propID}});

        return conn->getLastInsertedRowId();
    }

    snowflake ValuesDAO::addObject(int propID, snowflake objID) {
        if (bufferDependentObject.Size() + bufferIndependentObject.Size() > 0) {
            NLDB_WARN(
                "OBJECTS WERE STILL NOT ADDED, THIS INSERTION MIGHT FAIL.");
        }

        const std::string sql =
            "insert into @table (prop_id, obj_id) "
            "values (@prop_id, @obj_id);";

        conn->execute(
            sql,
            {{"@table", tables::getPropertyTypesTable()[PropertyType::OBJECT]},
             {"@prop_id", propID},
             {"@obj_id", objID}});

        return conn->getLastInsertedRowId();
    }

    snowflake ValuesDAO::deferAddObject(int propID) {
        snowflake id = SnowflakeGenerator::generate(0);

        auto val = BufferValueIndependentObject {.id = id, .prop_id = propID};

        if (!bufferIndependentObject.Add(val)) {
            this->pushPendingData();
            bufferIndependentObject.Add(val);
        }

        return id;
    }

    snowflake ValuesDAO::deferAddObject(int propID, snowflake objID) {
        snowflake id = SnowflakeGenerator::generate(0);

        auto val = BufferValueDependentObject {
            .id = id, .prop_id = propID, .obj_id = objID};

        if (!bufferDependentObject.Add(val)) {
            this->pushPendingData();
            bufferDependentObject.Add(val);
        }

        return id;
    }

    void ValuesDAO::updateStringLike(int propID, snowflake objID,
                                     PropertyType type, std::string value) {
        const std::string sql =
            "update @table set value = @prop_value where "
            "obj_id = @obj_id and prop_id = @prop_id;";

        conn->execute(
            utils::paramsbind::parseSQL(
                sql, {{"@table", tables::getPropertyTypesTable()[type]},
                      {"@prop_value", value},
                      {"@obj_id", objID},
                      {"@prop_id", propID}}),
            {});
    }

    bool ValuesDAO::exists(int propID, snowflake objID, PropertyType type) {
        const std::string sql =
            "select id from @table where prop_id = @prop_id and obj_id = "
            "@obj_id;";

        auto result = conn->executeAndGetFirstInt(
            utils::paramsbind::parseSQL(
                sql, {{"@table", tables::getPropertyTypesTable()[type]},
                      {"@obj_id", objID},
                      {"@prop_id", propID}}),
            {});

        return result.has_value();
    }

    std::optional<snowflake> ValuesDAO::findObjectId(int propID,
                                                     snowflake objID) {
        const std::string sql =
            "select id from @table where prop_id = @prop_id and obj_id = "
            "@obj_id;";

        return conn->executeAndGetFirstInt(
            utils::paramsbind::parseSQL(
                sql, {{"@table",
                       tables::getPropertyTypesTable()[PropertyType::OBJECT]},
                      {"@prop_id", propID},
                      {"@obj_id", objID}}),
            {});
    }

    void ValuesDAO::removeObject(snowflake objID) {
        std::stringstream sql;

        auto& tables = tables::getPropertyTypesTable();

        for (auto& [type, tab_name] : tables) {
            sql << "delete from " << tab_name << " where obj_id = " << objID
                << ";";
        }

        sql << "delete from " << tables[PropertyType::OBJECT]
            << " where id = " << objID << ";";

        conn->execute(sql.str(), {});
    }

    bool ValuesDAO::existsObject(snowflake objID) {
        const std::string sql = "select id from @table where id = @obj_id;";

        return conn
            ->executeAndGetFirstInt(
                utils::paramsbind::parseSQL(
                    sql,
                    {{"@table",
                      tables::getPropertyTypesTable()[PropertyType::OBJECT]},
                     {"@obj_id", objID}}),
                {})
            .has_value();
    }

    void ValuesDAO::deferAddStringLike(int propID, snowflake objID,
                                       PropertyType type, std::string value) {
        BufferValueStringLike val = {
            .propID = propID, .objID = objID, .type = type, .value = value};

        if (!bufferStringLike.Add(val)) {
            this->pushPendingData();
            bufferStringLike.Add(val);
        }
    }

    void ValuesDAO::pushPendingData() {
        NLDB_INFO("FLUSHING PENDING DATA");
        lock.lock();  // next push should wait

        auto& tables = tables::getPropertyTypesTable();

        /* -------------- INSERT INDEPENDET OBJECT -------------- */
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
            [&indep_sql, &tables](BufferValueIndependentObject& val,
                                  bool isLast) {
                indep_sql << "(" << val.id << ", " << val.prop_id << ")"
                          << (isLast ? ";" : ",");
            });

        const std::string indep_str = std::move(indep_sql).str();

        conn->execute(indep_str, {});

        bufferIndependentObject.Reset();

        /* --------------- INSERT DEPENDENT OBJECTS -------------- */
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
            [&dependent_sql, &tables](BufferValueDependentObject& val,
                                      bool isLast) {
                dependent_sql << "(" << val.id << "," << val.prop_id << ","
                              << val.obj_id << ")" << (isLast ? ";" : ",");
            });

        const std::string dependent_s = std::move(dependent_sql).str();

        conn->execute(dependent_s, {});

        bufferDependentObject.Reset();

        /* --------- INSERT DEPENDENT STRING LIKE VALUES -------- */

        std::map<PropertyType, std::stringstream> queries;
        bufferStringLike.ForEach(
            [&queries, &tables](BufferValueStringLike& val, bool isLast) {
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

        // combine them all and do it in one query or do 3 or 4 separated?
        for (auto& [p, subq] : queries) {
            sql << subq.rdbuf();
            // change last , for a ;
            sql.seekp(sql.tellp() - 1l);
            sql << ";";
        }

        const std::string depVal_s = std::move(sql).str();

        conn->execute(depVal_s, {});

        bufferStringLike.Reset();

        lock.unlock();
    }

    ValuesDAO::~ValuesDAO() { this->pushPendingData(); }
}  // namespace nldb