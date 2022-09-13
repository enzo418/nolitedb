#pragma once
#include "dbwrapper/IDB.hpp"

class Document {
   public:
    Document(int id);

   public:
    int getID();

   public:
    /**
     * @brief creates a document and return the id
     *
     * @param ctx
     * @param collectionID id of the collection the document belongs to
     * @return int
     */
    static int create(IDB& ctx, int collectionID);

   private:
    int id;
};