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

    // how many as you can fit in 0.333 MB
    constexpr int bufferSize = (int)(1e6 / 3 / sizeof(BufferValue));

    ValuesDAO::ValuesDAO(IDB* pConnection)
        : conn(pConnection), buffer(bufferSize) {}

    void ValuesDAO::addStringLike(int propID, int objID, PropertyType type,
                                  std::string value) {
        const std::string sql =
            "insert into @table (obj_id, prop_id, value) values (@obj_id, "
            "@prop_id, @value);";

        conn->execute(sql, {{"@table", tables::getPropertyTypesTable()[type]},
                            {"@obj_id", objID},
                            {"@prop_id", propID},
                            {"@value", value}});
    }

    int ValuesDAO::addObject(int propID) {
        const std::string sql =
            "insert into @table (prop_id) "
            "values (@prop_id);";

        conn->execute(
            sql,
            {{"@table", tables::getPropertyTypesTable()[PropertyType::OBJECT]},
             {"@prop_id", propID}});

        return conn->getLastInsertedRowId();
    }

    int ValuesDAO::addObject(int propID, int objID) {
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

    void ValuesDAO::updateStringLike(int propID, int objID, PropertyType type,
                                     std::string value) {
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

    bool ValuesDAO::exists(int propID, int objID, PropertyType type) {
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

    std::optional<int> ValuesDAO::findObjectId(int propID, int objID) {
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

    void ValuesDAO::removeObject(int objID) {
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

    bool ValuesDAO::existsObject(int objID) {
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

    void ValuesDAO::deferAddStringLike(int propID, int objID, PropertyType type,
                                       std::string value) {
        BufferValue val = {
            .propID = propID, .objID = objID, .type = type, .value = value};

        if (!buffer.Add(val)) {
            this->pushPendingData();
        }

        buffer.Add(val);
    }

    void ValuesDAO::pushPendingData() {
        NLDB_INFO("FLUSHING PENDING DATA");

        // TODO: lock guard

        std::stringstream sql;

        auto& tables = tables::getPropertyTypesTable();
        std::map<PropertyType, std::stringstream> queries;

        buffer.ForEach(
            [&sql, &queries, &tables](BufferValue& val, bool isLast) {
                if (!queries.contains(val.type)) {
                    queries[val.type] = {};
                    queries[val.type] << utils::paramsbind::parseSQL(
                        "insert into @table (prop_id, obj_id, value) values ",
                        {{"@table", tables[val.type]}});
                }

                queries[val.type]
                    << "(" << val.propID << ", " << val.objID << ", "
                    << utils::paramsbind::encloseQuotesConst(val.value) << "),";
            });

        // combine them all and do it in one query or do 3 or 4 separated?
        for (auto& [p, subq] : queries) {
            sql << subq.rdbuf();
            // change last , for a ;
            sql.seekp(sql.tellp() - 1l);
            sql << ";";
        }

        const std::string s = std::move(sql).str();

        conn->execute(s, {});

        buffer.Reset();
    }

    ValuesDAO::~ValuesDAO() { this->pushPendingData(); }
}  // namespace nldb