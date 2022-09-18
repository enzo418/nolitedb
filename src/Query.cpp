#include "Query.hpp"

#include <algorithm>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>

#include "Document.hpp"
#include "Enums.hpp"
#include "PropertyRep.hpp"
#include "SqlExpression.hpp"
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

int Query::update(int documentID, json updatedProperties) {
    qctx->resetQuery();

    auto& cl = this->qctx->cl;
    auto& sql = this->qctx->sql;
    auto& binds = this->qctx->bind;

    if (!cl.documentExists(documentID))
        throw std::runtime_error("Document doesn't exists");

    if (!updatedProperties.is_object())
        throw std::runtime_error("JSON object is not an object");

    for (auto& [propName, value] : updatedProperties.items()) {
        PropertyType propType = mapJsonType(value.type());
        bool isNewProperty = false;

        if (!cl.hasProperty(propName)) {
            cl.addProperty(propName, propType);
            isNewProperty = true;
        }

        int propertyID = cl.getProperty(propName).getId();

        binds = {
            {"@table_name", PropertyRep::getTableNameForTypeValue(propType)},
            {"@doc_id", documentID},
            {"@prop_id", propertyID}};

        if (propType == PropertyType::INTEGER) {
            binds.insert({"@prop_value", value.get<int>()});
        } else if (propType == PropertyType::DOUBLE) {
            binds.insert({"@prop_value", value.get<double>()});
        } else if (propType == PropertyType::STRING) {
            binds.insert({"@prop_value", value.get<std::string>()});
        }

        if (isNewProperty) {
            sql << utils::paramsbind::parseSQL(
                "insert into @table_name (doc_id, prop_id, value) values "
                "(@doc_id, @prop_id, @prop_value);",
                binds);
        } else {
            sql << utils::paramsbind::parseSQL(
                "update @table_name as pr set value = @prop_value where "
                "doc_id = @doc_id and prop_id = @prop_id;",
                binds);
        }
    }

    return qctx->db->executeMultipleOnOneStepRaw(sql.str(), binds);
}

int Query::insert(const json& obj) {
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

    return qctx->db->executeMultipleOnOneStepRaw(sql.str(), qctx->bind);
}

int Query::remove(int documentID) {
    qctx->resetQuery();
    auto& sql = qctx->sql;

    auto& tables = tables::getPropertyTables();

    sql << "delete from document where id = @id; ";

    for (auto& [type, tab_name] : tables) {
        sql << "delete from " << tab_name << " where doc_id = @id;";
    }

    return qctx->db->executeMultipleOnOneStepRaw(sql.str(),
                                                 {{"@id", documentID}});
}

Query QueryFactory::create(IDB* ctx, const std::string& collName) {
    return Query(
        std::make_shared<QueryCtx>(ctx, Collection::find(ctx, collName)));
}

SelectQuery::SelectQuery(const std::shared_ptr<QueryCtx>& pCtx,
                         std::vector<SelectProperty>&& pProperties)
    : ExecutableQuery<json>(pCtx, nullptr), properties(pProperties) {
    qctx->selectCtx->addFromClause();

    // add the joins needed for the select clause, where, order by,... are added
    // by them
    qctx->selectCtx->addJoinClauses(this->properties);

    this->setExecutableFunction([this](QueryCtx& ctx) {
        ctx.buildSelectQuery();  // sets ctx.sql

        json result = json::array();

        auto reader = ctx.db->executeReader(ctx.sql.str(), ctx.bind);

        auto& props = this->properties;
        std::shared_ptr<IDBRowReader> row;
        while (reader->readRow(row)) {
            json jrow = json::object();

            // each row will have exactly props.size() columns, and each one
            // will have the same type as the property in its index.
            for (int i = 0; i < props.size(); i++) {
                auto& v_prop = props[i];

                if (std::holds_alternative<PropertyRep>(v_prop)) {
                    const auto& prop = std::get<PropertyRep>(v_prop);

                    if (row->isNull(i)) {
                        // Should we set the field to null?
                        // jrow[prop.getName()] = nullptr;
                        continue;
                    }

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
                        case PropertyType::ID:
                            jrow["id"] = row->readInt32(i);
                            break;
                        default:
                            throw std::runtime_error(
                                "Select properties should not hold a reserved "
                                "property directly!");
                            break;
                    }
                } else {
                    auto& prop = std::get<AggregateFunction>(v_prop);
                    jrow[prop.alias] = row->readInt64(i);
                }
            }

            result.push_back(jrow);
        }

        return result;
    });
}

void SelectQueryData::addFromClause() {
    from_join << " from \"document\" as " << g_documentTableAlias << " ";
}

bool SelectQueryData::propAlreadyHasJoin(PropertyRep& prop) {
    // I overloaded the == operator! No worries
    return std::find_if(propsWithJoin.begin(), propsWithJoin.end(),
                        [&prop](PropertyRep itProp) {
                            return itProp.getId() == prop.getId();
                        }) != propsWithJoin.end();
}

void SelectQueryData::addJoinClauseIfNotExists(PropertyRep& prop) {
    if (!this->propAlreadyHasJoin(prop) && prop.getType() != PropertyType::ID) {
        from_join << utils::paramsbind::parseSQL(
            " left join @value_table as @p_a on (@doc_alias.id = "
            "@p_a.doc_id and @p_a.prop_id = @p_id)",
            {{"@value_table", prop.getTableNameForTypeValue(prop.getType())},
             {"@p_a", prop.getStatement()},
             {"@p_id", prop.getId()},
             {"@doc_alias", g_documentTableAlias}});
        this->propsWithJoin.push_back(prop);  // yes, copy it
    }
}

void SelectQueryData::addJoinClauses(std::vector<PropertyRep>& props) {
    for (auto& prop : props) {
        this->addJoinClauseIfNotExists(prop);
    }
}

void SelectQueryData::addJoinClauses(const std::set<PropertyRep*>& props) {
    for (const auto& prop : props) {
        this->addJoinClauseIfNotExists(*prop);
    }
}

void SelectQueryData::addJoinClauses(std::vector<SelectProperty>& props) {
    for (auto& v_prop : props) {
        if (std::holds_alternative<PropertyRep>(v_prop)) {
            addJoinClauseIfNotExists(std::get<PropertyRep>(v_prop));
        } else {
            addJoinClauseIfNotExists(*std::get<AggregateFunction>(v_prop).prop);
        }
    }
}

SelectQuery& SelectQuery::where(const SqlLogicExpression& st) {
    qctx->selectCtx->addJoinClauses(st.getActingProps());

    this->qctx->selectCtx->where << " where " << st.getStatement();
    return *this;
}

SelectQuery& SelectQuery::page(int pageNumber, int elementsPerPage) {
    this->qctx->selectCtx->limit_offset << " limit " << elementsPerPage
                                        << " offset "
                                        << (pageNumber - 1) * elementsPerPage;
    return *this;
}