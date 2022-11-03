#include "nldb/Query/QueryRunner.hpp"

#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>

#include "magic_enum.hpp"
#include "nldb/Collection.hpp"
#include "nldb/Common.hpp"
#include "nldb/DAL/Repositories.hpp"
#include "nldb/Exceptions.hpp"
#include "nldb/LOG/log.hpp"
#include "nldb/Profiling/Profiler.hpp"
#include "nldb/Property/Property.hpp"
#include "nldb/Property/SortedProperty.hpp"
#include "nldb/Query/QueryContext.hpp"
#include "nldb/Utils/ParamsBindHelpers.hpp"
#include "nldb/Utils/Variant.hpp"
#include "nldb/nldb_json.hpp"
#include "nldb/typedef.hpp"

namespace nldb {
    using namespace nldb::common;

    QueryRunner::QueryRunner(IDB* pConnection,
                             std::shared_ptr<Repositories> repos)
        : connection(pConnection), repos(repos) {}

    void QueryRunner::populateData(Collection& coll) {
        auto found = repos->repositoryCollection->find(coll.getName());
        if (found)
            coll = found.value();
        else
            throw CollectionNotFound(coll.getName());
    }

    void QueryRunner::populateData(Property& prop) {
        if (!prop.getParentCollName().has_value()) {
            // its a root property of a root collection, the name of the
            // property is the same as the root collection name
            auto coll = repos->repositoryCollection->find(prop.getName());
            if (!coll) {
                throw CollectionNotFound(prop.getName());
            }

            // all the collections have an owner id
            auto ownerID =
                repos->repositoryCollection->getOwnerId(coll->getId()).value();

            auto found = repos->repositoryProperty->find(ownerID);

            if (!found) {
                throw PropertyNotFound(prop.getName());
            }

            prop = found.value();
        } else {
            auto collName = prop.getParentCollName().value();
            snowflake parentID;

            if (prop.isParentNameAnExpression()) {
                // first parse the expression and get the parent coll id
                parentID = getLastCollectionIdFromExpression(collName);
            } else {
                // find the collection by its name
                auto found = repos->repositoryCollection->find(collName);

                if (found)
                    parentID = found.value().getId();
                else
                    throw CollectionNotFound(collName);
            }

            auto found =
                repos->repositoryProperty->find(parentID, prop.getName());

            if (found)
                prop = found.value();
            else
                throw PropertyNotFound(prop.getName());
        }
    }

    void QueryRunner::update(QueryPlannerContextUpdate&& data) {
        NLDB_PROFILE_BEGIN_SESSION("update", "nldb-profile-update.json");

        {
            NLDB_PROFILE_FUNCTION();
            populateData(data);

            // check if doc exists
            snowflake& docID = data.documentID;
            if (!repos->valuesDAO->existsObject(docID)) {
                throw DocumentNotFound("id: " + std::to_string(docID));
            }

            // get the collection
            auto& from = *data.from.begin();

            updateDocumentRecursive(docID, from, data.object);

            repos->pushPendingData();
        }

        NLDB_PROFILE_END_SESSION();
    }

    void QueryRunner::insert(QueryPlannerContextInsert&& data) {
        NLDB_PROFILE_BEGIN_SESSION("insert", "nldb-profile-insert.json");

        {
            NLDB_PROFILE_FUNCTION();

            NLDB_ASSERT(data.from.size() > 0, "missing target collection");

            if (data.from.size() > 1) {
                NLDB_WARN("multiple collections given in an insert clause");
            }

            try {
                populateData(data);
            } catch (CollectionNotFound& e) {
                // expected, do nothing
            }

            Collection& from = *data.from.begin();

            if (data.documents.is_array()) {
                for (auto& doc : data.documents) {
#ifdef NLDB_DEBUG_QUERY
                    NLDB_TRACE("INSERTING {}", doc.dump(2));
#endif
                    insertDocumentRecursive(doc, from.getName());
                }
            } else {
                insertDocumentRecursive(data.documents, from.getName());
            }

            {
                NLDB_PROFILE_SCOPE("Flush data");
                repos->pushPendingData();
            }
        }

        NLDB_PROFILE_END_SESSION();
    }

    void QueryRunner::remove(QueryPlannerContextRemove&& data) {
        populateData(data);

        repos->valuesDAO->removeObject(data.documentID);
    }

    auto GetCollIdOrCreateIt(const std::string& collName, Repositories* repos,
                             std::optional<snowflake> pRootPropID) {
        snowflake newCollId = -1;
        snowflake rootPropID = pRootPropID.value_or(-1);

        if (auto val = repos->repositoryCollection->find(collName)) {
            newCollId = val->getId();
            rootPropID =
                repos->repositoryCollection->getOwnerId(newCollId).value();
        } else {
            NLDB_TRACE("added missing collection '{}'", collName);
            rootPropID = pRootPropID.has_value()
                             ? pRootPropID.value()
                             : repos->repositoryProperty->add(collName);

            newCollId = repos->repositoryCollection->add(collName, rootPropID);
        }

        return std::array<snowflake, 2> {newCollId, rootPropID};
    }

    void QueryRunner::insertDocumentRecursive(
        json& doc, const std::string& collName,
        std::optional<snowflake> parentObjID,
        std::optional<snowflake> pRootPropID) {
        /**
         * rootPropID explanation: imagine we are inserting into persona the
         * object {name: "a", contact: {phone: 123}}, here we iterate
         * through the name and contact properties, for "name" create a
         * property of type string and for "contact" one of type object,
         * that will be the parent property of the new collection
         * "contact" that will be created in the next
         * `insertDocumentRecursive` call, that is way we pass the new
         * "contact" property to that function, so it knows the parent
         * prop.
         */

        NLDB_PROFILE_FUNCTION();

        //  - Add the collection if missing
        auto [collID, rootPropID] =
            GetCollIdOrCreateIt(collName, repos.get(), pRootPropID);

        //  - Create document/object
        snowflake objID =
            parentObjID ?  //
                repos->valuesDAO->addObject(rootPropID, parentObjID.value())
                        : repos->valuesDAO->addObject(rootPropID);

        for (auto& [propertyName, value] : doc.items()) {
            snowflake propID = -1;
            PropertyType type = JsonTypeToPropertyType((int)value.type());

            // skip null values, because if we set it to null then the cannot
            // change it (we can but it makes more sense to do it like this)
            if (type == PropertyType::_NULL) continue;

            //  - Create missing collection properties
            if (auto prop =
                    repos->repositoryProperty->find(collID, propertyName)) {
                if (prop->getType() != type) {
                    throw WrongPropertyType(
                        propertyName,
                        std::string(magic_enum::enum_name(prop->getType())),
                        std::string(magic_enum::enum_name(type)));
                }

                propID = prop->getId();
            } else {
                propID =
                    repos->repositoryProperty->add(propertyName, collID, type);
            }

            if (type == PropertyType::OBJECT) {
                //  - if property.type is Object
                //      - create a sub-collection and add a document/object
                //      - repeat the steps from the start

                this->insertDocumentRecursive(
                    value, getSubCollectionName(collName, propertyName), objID,
                    propID);
            } else {
                repos->valuesDAO->addStringLike(propID, objID, type,
                                                ValueToString(value));
            }
        }
    }

    void QueryRunner::updateDocumentRecursive(snowflake objID,
                                              const Collection& collection,
                                              json& object) {
        NLDB_PROFILE_FUNCTION();
        for (auto& [propName, valueJson] : object.items()) {
            std::optional<Property> found =
                repos->repositoryProperty->find(collection.getId(), propName);

            auto type = JsonTypeToPropertyType((int)valueJson.type());

            // maybe we should delete the value?
            if (type == PropertyType::_NULL) continue;

            snowflake propID;

            if (found.has_value()) {
                if (found->getType() != type) {
                    throw WrongPropertyType(
                        propName,
                        std::string(magic_enum::enum_name(found->getType())),
                        std::string(magic_enum::enum_name(type)));
                }

                propID = found->getId();
            } else {
                propID = repos->repositoryProperty->add(
                    propName, collection.getId(), type);
            }

            if (type != PropertyType::OBJECT) {
                std::string valueString = ValueToString(valueJson);

                // update the value or create a new one
                if (repos->valuesDAO->exists(propID, objID, type)) {
                    repos->valuesDAO->updateStringLike(propID, objID, type,
                                                       valueString);
                } else {
                    repos->valuesDAO->addStringLike(propID, objID, type,
                                                    valueString);
                }
            } else {
                // If the object property points has a value, update that
                // sub-document. Else a new value object should be added.

                auto childObjectID =
                    repos->valuesDAO->findObjectId(propID, objID);
                if (childObjectID.has_value()) {
                    auto chilCollection =
                        repos->repositoryCollection->findByOwner(propID);

                    updateDocumentRecursive(childObjectID.value(),
                                            chilCollection.value(),
                                            valueJson[propName]);
                } else {
                    // is a new object (sub-collection) of propID
                    // to get the collection we use the same format as the
                    // insert
                    auto subCollName =
                        getSubCollectionName(collection.getName(), propName);
                    // we need to get it before we create a new one because
                    // maybe another document of this collection already added a
                    // sub-collection for that property.
                    auto [subCollID, p] =
                        GetCollIdOrCreateIt(subCollName, repos.get(), propID);
                    // if we find it, we will extend it with more properties

                    // add a new document/object
                    auto childObjID = repos->valuesDAO->addObject(p, objID);

                    // add the new properties/values and update the existing
                    updateDocumentRecursive(childObjID,
                                            Collection(subCollID, subCollName),
                                            valueJson[propName]);
                }
            }
        }
    }

    snowflake QueryRunner::getLastCollectionIdFromExpression(
        const std::string& expr) {
        unsigned long last_pos = 0;
        snowflake id {-1};
        std::string name;
        auto len = expr.length();

        while (last_pos < len) {
            int pos = expr.find('.', last_pos);

            if (pos != expr.npos) {
                name = expr.substr(last_pos, pos - last_pos);
                last_pos = pos + 1;
            } else if (last_pos < len - 1) {
                name = expr.substr(last_pos, last_pos - len);
                last_pos = len;
            }

            if (id == -1) {
                // the first collection is the root collection, that means we
                // can find it by its name.
                auto found = repos->repositoryCollection->find(name);
                if (found)
                    id = found->getId();
                else
                    throw CollectionNotFound(name);
            } else {
                // iteration is not the first, we cannot find the collection by
                // its name but we can do it by first finding the parent node (a
                // property) and then the collection owned by this node.
                snowflake parentPropId {-1};
                auto propFound = repos->repositoryProperty->find(id, name);

                if (propFound)
                    parentPropId = propFound->getId();
                else
                    throw CollectionNotFound(name);  // yes

                auto found =
                    repos->repositoryCollection->findByOwner(parentPropId);
                if (found)
                    id = found->getId();
                else
                    throw CollectionNotFound(name);
            }
        }

        return id;
    }

    void QueryRunner::populateData(QueryPlannerContext& data) {
        for (auto& coll : data.from) {
            populateData(coll);
        }
    }

    void QueryRunner::populateData(QueryPlannerContextInsert& data) {
        populateData((QueryPlannerContext&)data);
    }

    void QueryRunner::populateData(QueryPlannerContextRemove& data) {
        populateData((QueryPlannerContext&)data);
    }

    void QueryRunner::populateData(QueryPlannerContextUpdate& data) {
        populateData((QueryPlannerContext&)data);
    }

    void QueryRunner::populateData(Object& obj) {
        auto& prop = obj.getPropertyRef();

        populateData(prop);

        obj.setCollId(
            repos->repositoryCollection->findByOwner(prop.getId())->getId());

        for (auto& prop : obj.getPropertiesRef()) {
            std::visit([this](auto& p) { populateData(p); }, prop);
        }
    }

    void QueryRunner::populateData(PropertyExpression& obj) {
        auto cbConst =
            overloaded {[this](Property& p) { populateData(p); }, [](auto&) {}};

        auto cb = overloaded {
            [this](box<PropertyExpression>& exp) { populateData(*exp); },
            [this, &cbConst](auto& cv) { std::visit(cbConst, cv); }};

        std::visit(cb, obj.left);
        std::visit(cb, obj.right);
    }

    void QueryRunner::populateData(QueryPlannerContextSelect& data) {
        NLDB_PROFILE_FUNCTION();

        populateData((QueryPlannerContext&)data);
        auto cb = overloaded {
            [this](auto& prop) { populateData(prop); },
            [this](AggregatedProperty& ag) { populateData(ag.property); }};

        /// select
        for (auto& prop : data.select_value) {
            std::visit(cb, prop);
        }

        // where
        if (data.where_value) cb(data.where_value.value());

        // group
        for (auto& prop : data.groupBy_value) {
            populateData(prop);
        }

        // sort
        for (auto& s : data.sortBy_value) {
            populateData(s.property);
        }

        // suppress
        for (auto& prop : data.suppress_value) {
            populateData(prop);
        }
    }
}  // namespace nldb