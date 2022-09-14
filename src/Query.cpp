#include "Query.hpp"

#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

#include "Document.hpp"
#include "Enums.hpp"
#include "dbwrapper/ParamsBind.hpp"
#include "logger/Logger.h"

PropertyType mapJsonType(json::value_t t) {
    switch (t) {
        case json::value_t::number_integer:
            return PropertyType::INTEGER;
        case json::value_t::number_float:
            return PropertyType::DOUBLE;
        case json::value_t::string:
            return PropertyType::STRING;
        default:
            auto msg = "Type is not supported" + std::to_string((int)t);
            throw std::runtime_error(msg);
    }
}

BaseQuery::BaseQuery(const std::shared_ptr<QueryCtx>& ctx) : qctx(ctx) {}

Query::Query(const std::shared_ptr<QueryCtx>& ctx) : BaseQuery(ctx) {}

void Query::buildPropertyInsert(
    std::stringstream& sql, Paramsbind& bind, json element,
    std::map<PropertyType, std::vector<std::string>>& insertMap) {
    auto& cl = qctx->cl;

    int docId = Document::create(*qctx->db, cl.getID());

    // iterate properties
    for (auto& [key, value] : element.items()) {
        PropertyType propType = mapJsonType(value.type());
        if (!cl.hasProperty(key)) {
            cl.addProperty(key, propType);
        }

        int propertyID = cl.getProperty(key).getId();

        auto bindValue = "@value_" + std::to_string(docId) + "_" +
                         std::to_string(propertyID);

        if (!insertMap.contains(propType)) {
            insertMap.insert({propType, {}});
        }

        sql << "(" << docId << "," << propertyID << "," << bindValue << ")";

        insertMap.at(propType).push_back(sql.str());

        if (propType == PropertyType::INTEGER) {
            bind.insert({bindValue, value.get<int>()});
        } else if (propType == PropertyType::DOUBLE) {
            bind.insert({bindValue, value.get<double>()});
        } else if (propType == PropertyType::STRING) {
            bind.insert({bindValue, value.get<std::string>()});
        }

        sql.str("");
    }
}

ExecutableQuery<int> Query::insert(const json& obj) {
    qctx->resetQuery();

    auto& sql = qctx->sql;
    std::map<PropertyType, std::vector<std::string>> valuesInsert;

    if (obj.is_array()) {
        for (auto& element : obj) {
            buildPropertyInsert(sql, qctx->bind, element, valuesInsert);
        }
    } else {
        buildPropertyInsert(sql, qctx->bind, obj, valuesInsert);
    }

    sql.str("");  // clear

    // build query based on the values
    for (auto& property : valuesInsert) {
        sql << "insert into "
            << PropertyRep::getTableNameForTypeValue(property.first)
            << " (doc_id, prop_id, value) "
            << " values ";
        for (const auto& insertValue : property.second) {
            sql << insertValue << ",";
        }
        sql.seekp(sql.tellp() - 1l);
        sql << ";";
    }

    return ExecutableQuery<int>(qctx, [](QueryCtx& ctx) {
        int affected =
            ctx.db->executeMultipleOnOneStepRaw(ctx.sql.str(), ctx.bind);

        if (affected <= 0) {
            LogWarning("Items might has not been added");
        }

        return affected;
    });
}

Query QueryFactory::create(IDB* ctx, const std::string& collName) {
    return Query(
        std::make_shared<QueryCtx>(ctx, Collection::find(ctx, collName)));
}

SelectQuery::SelectQuery(const std::shared_ptr<QueryCtx>& pCtx,
                         std::vector<PropertyRep>&& pProperties)
    : ExecutableQuery<json>(pCtx, nullptr), properties(pProperties) {
    this->addFromClause();

    // add the joins needed for the select clause, where, order by,... are added
    // by them
    this->addJoinClauses(this->properties);

    this->setExecutableFunction([this](QueryCtx& ctx) {
        json result = json::array();

        auto reader = ctx.db->executeReader(ctx.sql.str(), ctx.bind);

        auto& props = this->properties;
        std::shared_ptr<IDBRowReader> row;
        while (reader->readRow(row)) {
            json jrow = json::object();

            // each row will have exactly props.size() columns, and each one
            // will have the same type as the property in its index.
            for (int i = 0; i < props.size(); i++) {
                const auto& prop = props[i];
                switch (prop.getType()) {
                    case PropertyType::INTEGER:
                        jrow[prop.getName()] = row->readInt64(i);
                        break;
                    case PropertyType::DOUBLE:
                        jrow[prop.getName()] = row->readDouble(i);
                        break;
                    case PropertyType::STRING:
                        jrow[prop.getName()] = row->readString(i);
                        break;
                    case PropertyType::RESERVED:
                        throw std::runtime_error("Not implemented");
                        break;
                }
            }

            result.push_back(jrow);
        }

        return result;
    });
}

void SelectQuery::addFromClause() {
    this->qctx->sql << " from \"document\" as " << this->documentTableAlias
                    << " ";
}

void SelectQuery::addJoinClauses(std::vector<PropertyRep>& props) {
    for (auto& prop : props) {
        this->qctx->sql << utils::paramsbind::parseSQL(
            " left join @value_table as @p_a on (@doc_alias.id = "
            "@p_a.doc_id and @p_a.prop_id = @p_id)",
            {{"@value_table", prop.getTableNameForTypeValue(prop.getType())},
             {"@p_a", prop.getStatement()},
             {"@p_id", prop.getId()},
             {"@doc_alias", this->documentTableAlias}});
    }
}

SelectQuery& SelectQuery::where(const SqlStatement<std::string>& st) {
    this->qctx->sql << " where " << st.getStatement();
    return *this;
}
