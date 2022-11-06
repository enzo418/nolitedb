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
#include "nldb/LOG/log.hpp"
#include "nldb/LOG/managers/LogManagerSPD.hpp"
#include "nldb/Property/Property.hpp"
#include "nldb/Query/Query.hpp"
#include "nldb/SQL3Implementation.hpp"
#include "nldb/Utils/Enums.hpp"
#include "nldb/Utils/Variant.hpp"
#include "nldb/nldb_json.hpp"

using namespace nldb;

int main() {
    nldb::LogManager::Initialize();

    // optional, tweak the sqlite available memory.
    // With this example I reduced 70% of the memory usage.
    DBSL3 db(DBConfig {.page_size = 1000 * 16,
                       .page_cache_size = 1000 * 16,
                       .page_cache_N = 41});
    // DBSL3 db;

    remove("./twitter.db");

    if (!db.open("./twitter.db" /*":memory:"*/)) {
        std::cerr << "Could not open the database \n";
        db.throwLastError();
    }

    std::ifstream f("twitter.json");
    json data = json::parse(f)["statuses"];

    auto query = Query(&db, QueryConfiguration {
                                .PreferBuffer = true,
                                .SmallBufferSize = 1000, /*1 KB*/
                                .MediumBufferSize = 1000,
                                .LargeBufferSize = 1000 * 100 /*100 KB*/,
                            });

    Collection tweets = query.collection("tweets");

    auto now = std::chrono::high_resolution_clock::now();
    query.from(tweets).insert(data);

    db.logStatus();

    std::cout << "took "
              << (std::chrono::high_resolution_clock::now() - now) /
                     std::chrono::milliseconds(1)
              << " ms" << std::endl;

    now = std::chrono::high_resolution_clock::now();

    auto res =
        query.from("tweets")
            .select()
            .page(1, 5)
            //    try and compare:
            //    .suppress(tweets["user"], tweets["retweeted_status.user"])
            .execute();

    // std::cout << res << std::endl;

    std::cout << "size: " << res.size() << std::endl;

    std::cout << "select took "
              << (std::chrono::high_resolution_clock::now() - now) /
                     std::chrono::milliseconds(1)
              << " ms" << std::endl;

    db.logStatus();

    nldb::LogManager::Shutdown();
}
