#include "ValuesDAO.hpp"

#include <optional>

#include "backends/sqlite3/DAL/Definitions.hpp"
#include "nldb/LOG/log.hpp"
#include "nldb/Property/Property.hpp"
#include "nldb/Utils/ParamsBindHelpers.hpp"

namespace nldb {
    using namespace definitions;

    ValuesDAO::ValuesDAO(IDB* pConnection) : conn(pConnection) {}

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

    void ValuesDAO::removeAllObject(int objID) {
        std::stringstream sql;

        auto& tables = tables::getPropertyTypesTable();

        for (auto& [type, tab_name] : tables) {
            sql << "delete from " << tab_name << " where obj_id = " << objID
                << ";";
        }

        conn->execute(sql.str(), {});
    }
}  // namespace nldb