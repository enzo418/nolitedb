#include "Document.hpp"

Document::Document(int pId) : id(pId) {}

int Document::getID() { return this->id; }

int Document::create(IDB& ctx, int collectionID) {
    const std::string sql = "insert into document (coll_id) values (@id);";

    (void)ctx.executeOneStep(sql, {{"@id", collectionID}});

    return ctx.getLastInsertedRowId();
}