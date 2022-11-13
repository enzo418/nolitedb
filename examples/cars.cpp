#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

#include "backends/sqlite3/DB/DB.hpp"
#include "nldb/SQL3Implementation.hpp"
// #include "Enums.hpp"
#include "nldb/LOG/log.hpp"
#include "nldb/LOG/managers/LogManagerSPD.hpp"
#include "nldb/Property/Property.hpp"
#include "nldb/Query/Query.hpp"
// #include "nlohmann/json.hpp"
#include "nldb/Utils/Enums.hpp"
#include "nldb/Utils/Variant.hpp"
#include "nldb/nldb_json.hpp"
#include "nldb/typedef.hpp"

using namespace nldb;

int main() {
    nldb::LogManager::Initialize();

    // optional, tweak the sqlite available memory.
    DBSL3 db(DBConfig {
        .page_size = 2048, .page_cache_size = 6144, .page_cache_N = 3});
    // DBSL3 db;

    remove("./cars.db");

    if (!db.open("./cars.db" /*":memory:"*/)) {
        std::cerr << "Could not open the database \n";
        db.throwLastError();
    }

    // a query object can be used to execute multiple queries.
    Query query = Query(&db, QueryConfiguration {
                                 .PreferBuffer = false,
                                 .SmallBufferSize = 1000, /*1 KB*/
                                 .MediumBufferSize = 1000,
                                 .LargeBufferSize = 1000,
                             });

    // {name, founded, country}
    json data_automaker = {{{"name", "ford"},
                            {"founded", "June 16, 1903"},
                            {"country", "United States"}},
                           // -------------
                           {{"name", "subaru"},
                            {"founded", "July 15, 1953"},
                            {"country", "Japan"}}};

    // {maker, model, year}
    json data_cars = {
        {{"maker", "ford"}, {"model", "focus"}, {"year", 2011}},
        {{"maker", "ford"}, {"model", "focus"}, {"year", 2015}},
        {{"maker", "subaru"}, {"model", "impreza"}, {"year", 2003}}};

    // insert automakers
    query.from("automaker").insert(data_automaker);

    db.logStatus();

    // group all the automaker properties into the `automaker` variable.
    Collection automakers = query.collection("automaker");

    // we can obtain the properties from a collection before inserting any value
    // in it
    Collection cars = query.collection("cars");
    auto [id, model, maker, year] = cars.get("_id", "model", "maker", "year");

    // insert cars data
    query.from("cars").insert(data_cars);

    // select all the cars with manufacturer details
    json all = query.from("cars")
                   .select(id, model, maker, year, automakers)
                   .where(year > 1990 && automakers["name"] == maker)
                   .page(1, 10)
                   .suppress(automakers["_id"])
                   .execute();

    std::cout << "\n\nCars with automaker: " << all.dump(2) << std::endl
              << std::endl;

    auto automaker = automakers.group("_id", "name", "founded");

    // select id, model, maker and max year from each model
    auto res1 = query.from("cars")
                    .select(id, model, maker, year.maxAs("year_newest_model"),
                            automaker)
                    .where(year > 1990 && automaker["name"] == maker)
                    .page(1, 10)
                    .groupBy(model, maker)
                    .execute();

    std::cout << "\n\nNewest models with automaker details {name, founded}: "
              << res1.dump(2) << std::endl;

    // update first car, set year to 2100 and add a new
    // property called price
    snowflake id0 = std::stoll(res1[0]["_id"].get<std::string>());
    query.from("cars").update(id0, {{"year", 2100}, {"price", 50000}});

    auto res2 =
        query.from("cars").select().page(1, 10).sortBy(year.desc()).execute();

    std::cout << "\n\nUpdated: " << res2.dump(2) << std::endl;

    auto final = query.from("cars").select(model, maker, year).execute();

    std::cout << "\n\nBefore remove 1: " << final.dump(2) << std::endl;

    query.from("cars").remove(id0);

    auto finalThen = query.from("cars").select(model, maker, year).execute();

    std::cout << "\n\nAfter remove: " << finalThen.dump(2) << std::endl;

    db.logStatus();

    nldb::LogManager::Shutdown();
}
