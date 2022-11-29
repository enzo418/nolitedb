#include "nldb/backends/sqlite3/DAL/ValuesDAO.hpp"

#include <map>
#include <optional>
#include <sstream>
#include <unordered_map>

#include "magic_enum.hpp"
#include "nldb/LOG/log.hpp"
#include "nldb/Property/Property.hpp"
#include "nldb/Utils/ParamsBindHelpers.hpp"
#include "nldb/backends/sqlite3/DAL/Definitions.hpp"

namespace nldb {
    using namespace definitions;

    ValuesDAO::ValuesDAO(IDB* pConnection) : conn(pConnection) {}

    void ValuesDAO::addStringLike(snowflake propID, snowflake objID,
                                  PropertyType type, std::string value) {
        const std::string sql =
            "insert into @table (obj_id, prop_id, value) values (@obj_id, "
            "@prop_id, @value);";

        conn->execute(sql, {{"@table", tables::getPropertyTypesTable()[type]},
                            {"@obj_id", objID},
                            {"@prop_id", propID},
                            {"@value", value}});
    }

    snowflake ValuesDAO::addObject(snowflake propID,
                                   std::optional<snowflake> objID) {
        if (objID.has_value()) {
            const std::string sql =
                "insert into @table (prop_id, obj_id) "
                "values (@prop_id, @obj_id);";

            conn->execute(
                sql, {{"@table",
                       tables::getPropertyTypesTable()[PropertyType::OBJECT]},
                      {"@prop_id", propID},
                      {"@obj_id", objID.value()}});
        } else {
            const std::string sql =
                "insert into @table (prop_id) "
                "values (@prop_id);";

            conn->execute(
                sql, {{"@table",
                       tables::getPropertyTypesTable()[PropertyType::OBJECT]},
                      {"@prop_id", propID}});
        }

        return conn->getLastInsertedRowId();
    }

    snowflake ValuesDAO::addObjectWithID(snowflake id, snowflake propID,
                                         std::optional<snowflake> objID) {
        if (objID.has_value()) {
            const std::string sql =
                "insert into @table (id, prop_id, obj_id) "
                "values (@id, @prop_id, @obj_id);";

            conn->execute(
                sql, {{"@table",
                       tables::getPropertyTypesTable()[PropertyType::OBJECT]},
                      {"@prop_id", propID},
                      {"@obj_id", objID.value()},
                      {"@id", id}});
        } else {
            const std::string sql =
                "insert into @table (id, prop_id) values (@id, @prop_id);";

            conn->execute(
                sql, {{"@table",
                       tables::getPropertyTypesTable()[PropertyType::OBJECT]},
                      {"@prop_id", propID},
                      {"@id", id}});
        }

        return id;
    }

    void ValuesDAO::updateStringLike(snowflake propID, snowflake objID,
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

    bool ValuesDAO::exists(snowflake propID, snowflake objID,
                           PropertyType type) {
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

    std::optional<snowflake> ValuesDAO::findObjectId(snowflake propID,
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
}  // namespace nldb