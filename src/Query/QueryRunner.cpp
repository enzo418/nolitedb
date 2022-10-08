#include "nldb/Query/QueryRunner.hpp"

#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

#include "magic_enum.hpp"
#include "nldb/Collection.hpp"
#include "nldb/Common.hpp"
#include "nldb/Exceptions.hpp"
#include "nldb/LOG/log.hpp"
#include "nldb/Property/Property.hpp"
#include "nldb/Property/SortedProperty.hpp"
#include "nldb/Query/QueryContext.hpp"
#include "nldb/Utils/ParamsBindHelpers.hpp"
#include "nldb/nldb_json.hpp"

namespace nldb {
    using namespace nldb::common;

    int AddCollectionIfMissing(const std::string& collName,
                               Repositories& repos);

    QueryRunner::QueryRunner(IDB* pConnection) : connection(pConnection) {}

    json QueryRunner::select(QueryPlannerContextSelect&& data) {
        // TODO: make this function virtual
        NLDB_ASSERT(false, "Not implemented.");
    }

    void QueryRunner::update(QueryPlannerContextUpdate&& data) {
        auto& repos = data.repos;

        // check if doc exists
        int& docID = data.documentID;
        if (!repos.repositoryDocument->exists(docID)) {
            throw DocumentNotFound("id: " + std::to_string(docID));
        }

        // get the collection
        auto& from = *data.from.begin();

        std::optional<Collection> collection;
        if (from.hasCollectionAssigned()) {
            collection = (Collection)from;
        } else {
            collection = repos.repositoryCollection->find(from.getName());
        }

        if (!collection.has_value()) {
            throw CollectionNotFound(from.getName());
        }

        updateDocumentRecursive(docID, collection.value(), data.object, repos);
    }

    void QueryRunner::insert(QueryPlannerContextInsert&& data) {
        NLDB_ASSERT(data.from.size() > 0, "missing target collection");

        if (data.from.size() > 1) {
            NLDB_WARN("multiple collections given in an insert clause");
        }

        CollectionQuery& queryFrom = *data.from.begin();

        if (data.documents.is_array()) {
            for (auto& doc : data.documents) {
                std::cout << doc << std::endl;
                insertDocumentRecursive(doc, queryFrom.getName(), data.repos);
            }
        } else {
            insertDocumentRecursive(data.documents, queryFrom.getName(),
                                    data.repos);
        }
    }

    void QueryRunner::remove(QueryPlannerContextRemove&& data) {
        data.repos.repositoryDocument->remove(data.documentID);

        data.repos.valuesDAO->removeAllFromDocument(data.documentID);
    }

    int AddCollectionIfMissing(const std::string& collName,
                               Repositories& repos) {
        if (auto val = repos.repositoryCollection->find(collName)) {
            return val->getId();
        } else {
            NLDB_TRACE("added missing collection '{}'", collName);
            return repos.repositoryCollection->add(collName);
        }
    }

    std::pair<int, int> QueryRunner::insertDocumentRecursive(
        json& doc, const std::string& collName, Repositories& repos) {
        //  - Add collection if missing
        //  - Create document
        //  - Create missing collection properties
        //  - if property.type is Object
        //      - create a sub-collection with the name
        //      "_{collName}_{propertyName}"
        //      - repeat the steps from the start
        int collID = AddCollectionIfMissing(collName, repos);

        int docID = repos.repositoryDocument->add(collID);

        for (auto& [propertyName, value] : doc.items()) {
            int propID = -1;
            PropertyType type = JsonTypeToPropertyType((int)value.type());

            if (auto prop =
                    repos.repositoryProperty->find(collID, propertyName)) {
                if (prop->getType() != type) {
                    throw WrongPropertyType(
                        propertyName,
                        std::string(magic_enum::enum_name(prop->getType())),
                        std::string(magic_enum::enum_name(type)));
                }

                propID = prop->getId();
            } else {
                propID =
                    repos.repositoryProperty->add(propertyName, collID, type);
            }

            if (type == PropertyType::OBJECT) {
                auto [sub_coll_id, sub_doc_id] = this->insertDocumentRecursive(
                    value, getSubCollectionName(collName, propertyName), repos);
                repos.valuesDAO->addObject(propID, docID, sub_coll_id,
                                           sub_doc_id);
            } else {
                repos.valuesDAO->addStringLike(propID, docID, type,
                                               ValueToString(value));
            }
        }

        return {collID, docID};
    }

    void QueryRunner::updateDocumentRecursive(int docID,
                                              const Collection& collection,
                                              json& object,
                                              Repositories& repos) {
        for (auto& [propName, valueJson] : object.items()) {
            std::optional<Property> found =
                repos.repositoryProperty->find(collection.getId(), propName);

            auto type = JsonTypeToPropertyType((int)valueJson.type());

            int propID;

            if (found.has_value()) {
                if (found->getType() != type) {
                    throw WrongPropertyType(
                        propName,
                        std::string(magic_enum::enum_name(found->getType())),
                        std::string(magic_enum::enum_name(type)));
                }

                propID = found->getId();
            } else {
                propID = repos.repositoryProperty->add(
                    propName, collection.getId(), type);
            }

            if (type != PropertyType::OBJECT) {
                std::string valueString = ValueToString(valueJson);

                // update the value or create a new one
                if (repos.valuesDAO->exists(propID, docID, type)) {
                    repos.valuesDAO->updateStringLike(propID, docID, type,
                                                      valueString);
                } else {
                    repos.valuesDAO->addStringLike(propID, docID, type,
                                                   valueString);
                }
            } else {
                // If the object property points has a value, update that
                // sub-document. Else a new value object should be added.

                auto objectValue = repos.valuesDAO->findObject(propID, docID);
                if (objectValue.has_value()) {
                    auto collection = repos.repositoryCollection->find(
                        objectValue->sub_coll_id);

                    updateDocumentRecursive(objectValue->sub_doc_id,
                                            collection.value(), valueJson,
                                            repos);
                } else {
                    // is a new object (sub-collection) of propID
                    // to get the collection we use the same format as the
                    // insert
                    auto subCollName =
                        getSubCollectionName(collection.getName(), propName);
                    // we need to get it before we create a new one because
                    // maybe another document of this collection already added a
                    // sub-collection for that property.
                    auto subCollFound =
                        repos.repositoryCollection->find(subCollName);

                    int sub_coll_id;
                    if (subCollFound) {
                        // found it, we will extend it with more properties
                        sub_coll_id = subCollFound->getId();
                    } else {
                        // create a new sub-collection
                        sub_coll_id =
                            repos.repositoryCollection->add(subCollName);
                    }

                    int sub_doc_id = repos.repositoryDocument->add(sub_coll_id);

                    // add the new properties/values and update the existing
                    updateDocumentRecursive(
                        sub_doc_id, Collection(sub_coll_id, subCollName),
                        valueJson, repos);

                    // value didn't exist before, add it.
                    repos.valuesDAO->addObject(propID, docID, sub_coll_id,
                                               sub_doc_id);
                }
            }
        }
    }
}  // namespace nldb