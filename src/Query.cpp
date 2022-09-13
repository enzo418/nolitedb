#include "Query.hpp"

#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

#include "Document.hpp"
#include "Enums.hpp"
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

Query::Query(IDB* pCtx, const Collection& pCl) : ctx(pCtx), cl(pCl) {}

void CollectionQueryFactory::buildPropertyInsert(
    std::stringstream& sql, Paramsbind& bind, json element,
    std::map<PropertyType, std::vector<std::string>>& insertMap) {
    int docId = Document::create(*ctx, cl.getID());

    // iterate properties
    for (auto& [key, value] : element.items()) {
        PropertyType propType = mapJsonType(value.type());
        if (!cl.hasProperty(key)) {
            cl.addProperty(key, propType);
        }

        int propertyID = cl.getPropertyID(key).value();

        auto bindValue = "@value_" + std::to_string(docId);

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

void CollectionQueryFactory::insert(const json& obj) {
    std::stringstream sql;
    Paramsbind bind;
    std::map<PropertyType, std::vector<std::string>> valuesInsert;

    if (obj.is_array()) {
        for (auto& element : obj) {
            buildPropertyInsert(sql, bind, element, valuesInsert);
        }
    } else {
        buildPropertyInsert(sql, bind, obj, valuesInsert);
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

    int affected = ctx->executeMultipleOnOneStepRaw(sql.str(), bind);

    if (affected <= 0) {
        LogWarning("Items might has not been added");
    }
}

CollectionQueryFactory::CollectionQueryFactory(IDB* pCtx, const Collection& pCl)
    : ctx(pCtx), cl(pCl) {}

CollectionQueryFactory QueryFactory::create(IDB* ctx,
                                            const std::string& collName) {
    return CollectionQueryFactory(ctx, Collection::find(ctx, collName));
}