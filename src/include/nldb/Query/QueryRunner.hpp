#pragma once

#include "nldb/Collection.hpp"
#include "nldb/DAL/Repositories.hpp"
#include "nldb/DB/IDB.hpp"
#include "nldb/Exceptions.hpp"
#include "nldb/Profiling/Profiler.hpp"
#include "nldb/Property/Property.hpp"
#include "nldb/Query/IQueryRunner.hpp"
#include "nldb/Query/QueryContext.hpp"
#include "nldb/Utils/Variant.hpp"

namespace nldb {
    constexpr bool DoThrow = true;
    constexpr bool DoNotThrow = false;

    /**
     * @brief A repository oriented implementation of IQueryRunner.
     * Can be optimized by working in a more handcrafted implementation.
     */
    class QueryRunner : public IQueryRunner {
       public:
        QueryRunner(IDB* connection, std::shared_ptr<Repositories> repos);

       public:
        // virtual json select(QueryPlannerContextSelect&& data) override;
        virtual void update(QueryPlannerContextUpdate&& data) override;

        // inserts the documents and returns their ids
        virtual std::vector<std::string> insert(
            QueryPlannerContextInsert&& data) override;
        virtual void remove(QueryPlannerContextRemove&& data) override;

       protected:  // helpers runners
        /**
         * @brief inserts a new document and returns its id.
         * @param doc object to insert
         * @param collName target collection
         * @param repos repositories/data access objects
         * @param parentObjID parent object
         * @param rootPropID the parent property id for the new collection (if
         * missing).
         */
        virtual snowflake insertDocumentRecursive(
            json& doc, const std::string& collName,
            std::optional<snowflake> parentObjID = std::nullopt,
            std::optional<snowflake> rootPropID = std::nullopt);

        /**
         * @brief updates a document that can contain more documents (objects)
         * @param objID document to update
         * @param collection collection that the document belong to
         * @param object new/updated properties with their values
         * @param repos repositories/data access objects
         */
        virtual void updateDocumentRecursive(snowflake objID,
                                             const Collection& collection,
                                             json& object);

       protected:  // helpers data
        virtual snowflake getLastCollectionIdFromExpression(
            const std::string& expr);

        // adds the id it has in the database
        template <bool Throw>
        void populateData(Collection& coll) {
            auto found = repos->repositoryCollection->find(coll.getName());
            if (found)
                coll = found.value();
            else {
                if constexpr (Throw) {
                    throw CollectionNotFound(coll.getName());
                }
            }
        }

        template <bool Throw>
        void populateData(Property& prop) {
            if (!prop.getParentCollName().has_value()) {
                // its a root property of a root collection, the name of the
                // property is the same as the root collection name
                auto coll = repos->repositoryCollection->find(prop.getName());
                if (!coll) {
                    if constexpr (Throw) {
                        throw CollectionNotFound(prop.getName());
                    }
                }

                // all the collections have an owner id
                auto ownerID =
                    repos->repositoryCollection->getOwnerId(coll->getId())
                        .value();

                auto found = repos->repositoryProperty->find(ownerID);

                if (!found) {
                    if constexpr (Throw) {
                        throw PropertyNotFound(prop.getName());
                    }
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
                    else {
                        if constexpr (Throw) {
                            throw CollectionNotFound(collName);
                        }
                    }
                }

                auto found =
                    repos->repositoryProperty->find(parentID, prop.getName());

                if (found)
                    prop = found.value();
                else {
                    if constexpr (Throw) {
                        throw PropertyNotFound(prop.getName());
                    }
                }
            }
        }

        // contexts
        template <bool Throw>
        void populateData(QueryPlannerContext& data) {
            for (auto& coll : data.from) {
                populateData<Throw>(coll);
            }
        }

        template <bool Throw>
        void populateData(QueryPlannerContextInsert& data) {
            populateData<Throw>((QueryPlannerContext&)data);
        }

        template <bool Throw>
        void populateData(QueryPlannerContextRemove& data) {
            populateData<Throw>((QueryPlannerContext&)data);
        }

        template <bool Throw>
        void populateData(QueryPlannerContextUpdate& data) {
            populateData<Throw>((QueryPlannerContext&)data);
        }

        // recursive classes
        template <bool Throw>
        void populateData(Object& obj) {
            auto& prop = obj.getPropertyRef();

            populateData<Throw>(prop);

            obj.setCollId(repos->repositoryCollection->findByOwner(prop.getId())
                              ->getId());

            auto cb =
                overloaded {[this](auto& prop) { populateData<Throw>(prop); },
                            [this](AggregatedProperty& ag) {
                                populateData<Throw>(ag.property);
                            }};

            for (auto& prop : obj.getPropertiesRef()) {
                std::visit(cb, prop);
            }
        }

        template <bool Throw>
        void populateData(PropertyExpression& obj) {
            auto cbConst = overloaded {
                [this](Property& p) { populateData<Throw>(p); }, [](auto&) {}};

            auto cb =
                overloaded {[this](box<PropertyExpression>& exp) {
                                populateData<Throw>(*exp);
                            },
                            [&cbConst](auto& cv) { std::visit(cbConst, cv); }};

            std::visit(cb, obj.left);
            std::visit(cb, obj.right);
        }

        template <bool Throw>
        void populateData(QueryPlannerContextSelect& data) {
            NLDB_PROFILE_FUNCTION();

            populateData<Throw>((QueryPlannerContext&)data);
            auto cb =
                overloaded {[this](auto& prop) { populateData<Throw>(prop); },
                            [this](AggregatedProperty& ag) {
                                populateData<Throw>(ag.property);
                            }};

            /// select
            for (auto& prop : data.select_value) {
                std::visit(cb, prop);
            }

            // where
            if (data.where_value) cb(data.where_value.value());

            // group
            for (auto& prop : data.groupBy_value) {
                populateData<Throw>(prop);
            }

            // sort
            for (auto& s : data.sortBy_value) {
                populateData<Throw>(s.property);
            }

            // suppress
            for (auto& prop : data.suppress_value) {
                populateData<Throw>(prop);
            }
        }

       protected:
        IDB* connection;
        std::shared_ptr<Repositories> repos;
    };
}  // namespace nldb