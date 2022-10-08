#include "ValuesDAO.hpp"

#include <optional>

#include "backends/sqlite3/DAL/Definitions.hpp"
#include "nldb/LOG/log.hpp"
#include "nldb/Property/Property.hpp"
#include "nldb/Utils/ParamsBindHelpers.hpp"

namespace nldb {
    using namespace definitions;

    ValuesDAO::ValuesDAO(IDB* pConnection) : conn(pConnection) {}

    void ValuesDAO::addStringLike(int propID, int docID, PropertyType type,
                                  std::string value) {
        const std::string sql =
            "insert into @table (doc_id, prop_id, value) values (@doc_id, "
            "@prop_id, @value);";

        conn->execute(sql, {{"@table", tables::getPropertyTypesTable()[type]},
                            {"@doc_id", docID},
                            {"@prop_id", propID},
                            {"@value", value}});
    }

    void ValuesDAO::addObject(int propID, int docID, int subCollID,
                              int subDocId) {
        const std::string sql =
            "insert into @table (doc_id, prop_id, sub_coll_id, sub_doc_id) "
            "values (@doc_id, @prop_id, @sub_coll_id, @sub_doc_id);";

        conn->execute(
            sql,
            {{"@table", tables::getPropertyTypesTable()[PropertyType::OBJECT]},
             {"@doc_id", docID},
             {"@prop_id", propID},
             {"@sub_coll_id", subCollID},
             {"@sub_doc_id", subDocId}});
    }

    void ValuesDAO::updateStringLike(int propID, int docID, PropertyType type,
                                     std::string value) {
        const std::string sql =
            "update @table set value = @prop_value where "
            "doc_id = @doc_id and prop_id = @prop_id;";

        conn->execute(
            utils::paramsbind::parseSQL(
                sql, {{"@table", tables::getPropertyTypesTable()[type]},
                      {"@prop_value", value},
                      {"@doc_id", docID},
                      {"@prop_id", propID}}),
            {});
    }

    void ValuesDAO::updateObject(int propID, int docID, int subCollID,
                                 int subDocId) {
        NLDB_CRITICAL("MISSING IMPL");
    }

    bool ValuesDAO::exists(int propID, int docID, PropertyType type) {
        const std::string sql =
            "select id from @table where prop_id = @prop_id and doc_id = "
            "@doc_id;";

        auto result = conn->executeAndGetFirstInt(
            utils::paramsbind::parseSQL(
                sql, {{"@table", tables::getPropertyTypesTable()[type]},
                      {"@doc_id", docID},
                      {"@prop_id", propID}}),
            {});

        return result.has_value();
    }

    std::optional<ValueObjectMapped> ValuesDAO::findObject(int propID,
                                                           int docID) {
        const std::string sql =
            "select id, sub_coll_id, sub_doc_id from @table where prop_id = "
            "@prop_id and doc_id = "
            "@doc_id;";

        auto reader = conn->executeReader(
            utils::paramsbind::parseSQL(
                sql, {{"@table",
                       tables::getPropertyTypesTable()[PropertyType::OBJECT]},
                      {"@prop_id", propID},
                      {"@doc_id", docID}}),
            {});

        std::shared_ptr<IDBRowReader> row;
        while (reader->readRow(row)) {
            ValueObjectMapped obj = {.doc_id = docID, .prop_id = propID};
            obj.id = row->readInt32(0);
            obj.sub_coll_id = row->readInt32(1);
            obj.sub_doc_id = row->readInt32(2);
            return obj;
        }

        return std::nullopt;
    }

    std::optional<int> ValuesDAO::findSubCollectionOfObjectProperty(
        int propID) {
        const std::string sql =
            "select sub_coll_id from value_object where prop_id = @id;";
        return conn->executeAndGetFirstInt(sql, {{"@id", propID}});
    }

    void ValuesDAO::removeAllFromDocument(int docID) {
        std::stringstream sql;

        auto& tables = tables::getPropertyTypesTable();

        for (auto& [type, tab_name] : tables) {
            sql << "delete from " << tab_name << " where doc_id = " << docID
                << ";";
        }

        conn->execute(sql.str(), {});
    }
}  // namespace nldb