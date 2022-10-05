#include "nldb/DB/IDB.hpp"
#include "nldb/Query/IQueryRunner.hpp"

namespace nldb {
    /**
     * @brief A repository oriented implementation of IQueryRunner.
     * Can be optimized by working in a more handcrafted implementation.
     */
    class QueryRunner : public IQueryRunner {
       public:
        QueryRunner(IDB* connection);

       public:
        virtual json select(QueryPlannerContextSelect&& data) override;
        virtual void update(QueryPlannerContextUpdate&& data) override;
        virtual void insert(QueryPlannerContextInsert&& data) override;
        virtual void remove(QueryPlannerContextRemove&& data) override;

       protected:
        /**
         * @brief inserts a new document and returns its id.
         * @param doc object to insert
         * @param collName target collection
         * @param repos repositories/data access objects
         * @return std::pair<int, int> inserted in (collection_id, document_id)
         */
        virtual std::pair<int, int> insertDocumentRecursive(
            json& doc, const std::string& collName, Repositories& repos);

        /**
         * @brief updates a document that can contain more documents (objects)
         * @param docID document to update
         * @param collection collection that the document belong to
         * @param object new/updated properties with their values
         * @param repos repositories/data access objects
         */
        virtual void updateDocumentRecursive(int docID,
                                             const Collection& collection,
                                             json& object, Repositories& repos);

       protected:
        IDB* connection;
    };
}  // namespace nldb