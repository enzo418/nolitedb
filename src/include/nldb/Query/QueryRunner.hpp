#pragma once

#include "nldb/Collection.hpp"
#include "nldb/DAL/Repositories.hpp"
#include "nldb/DB/IDB.hpp"
#include "nldb/Property/Property.hpp"
#include "nldb/Query/IQueryRunner.hpp"
#include "nldb/Query/QueryContext.hpp"

namespace nldb {
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
        virtual void insert(QueryPlannerContextInsert&& data) override;
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
        virtual void insertDocumentRecursive(
            json& doc, const std::string& collName,
            std::optional<snowflake> parentObjID = std::nullopt,
            std::optional<int> rootPropID = std::nullopt);

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
        virtual int getLastCollectionIdFromExpression(const std::string& expr);

        // adds the id it has in the database
        virtual void populateData(Collection& coll);
        virtual void populateData(Property& prop);

        // recursive classes
        void populateData(Object& prop);
        void populateData(PropertyExpression& prop);

        // contexts
        void populateData(QueryPlannerContext& data);
        void populateData(QueryPlannerContextInsert& data);
        void populateData(QueryPlannerContextRemove& data);
        void populateData(QueryPlannerContextUpdate& data);
        void populateData(QueryPlannerContextSelect& data);

       protected:
        IDB* connection;
        std::shared_ptr<Repositories> repos;
    };
}  // namespace nldb