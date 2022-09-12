#include "Collection.hpp"

#include <stdexcept>

#include "dbwrapper/IDB.hpp"

Collection::Collection(int id, const std::string& name) {}

Collection Collection::find(IDB& ctx, const std::string& name) {
    const std::string sql = "SELECT id FROM collections where name = @name";

    auto query = ctx.execute(sql, {{"name", name}});
    std::shared_ptr<IDBRowReader> row;
    int id {-1};
    while (query->readRow(row)) {
        id = row->readInt32(0);
        break;
    }

    if (id != -1) {
        return Collection(id, name);
    } else {
        return Collection(create(ctx, name), name);
    }
}

int Collection::create(IDB& ctx, const std::string& name) {
    const std::string sql = "insert into collection (name) values (@name);";

    (void)ctx.execute(sql, {{"name", name}});

    return ctx.getLastInsertedRowId();
}