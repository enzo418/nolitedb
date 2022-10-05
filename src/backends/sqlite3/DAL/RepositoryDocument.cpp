#include "RepositoryDocument.hpp"

#include <sstream>

#include "Definitions.hpp"
#include "nldb/Collection.hpp"
#include "nldb/DAL/Repositories.hpp"
#include "nldb/Exceptions.hpp"
#include "nldb/LOG/log.hpp"
#include "nldb/Query/QueryContext.hpp"

namespace nldb {
    using namespace nldb::definitions;

    RepositoryDocument::RepositoryDocument(IDB* connection)
        : conn(connection) {}

    int RepositoryDocument::add(int collectionID) {
        const std::string sql = "insert into document (coll_id) values (@id);";

        conn->execute(sql, {{"@id", collectionID}});

        return conn->getLastInsertedRowId();
    }

    void RepositoryDocument::remove(int id) {
        const std::string sql = "delete from document where id = @id;";

        conn->execute(sql, {{"@id", id}});
    }

    bool RepositoryDocument::exists(int id) {
        const std::string sql = "select id from document where id = @id;";

        return conn->executeAndGetFirstInt(sql, {{"@id", id}}).has_value();
    }
}  // namespace nldb