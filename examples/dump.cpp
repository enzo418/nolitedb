#include <chrono>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

#include "nldb/LOG/log.hpp"
#include "nldb/Property/Property.hpp"
#include "nldb/Query/Query.hpp"
#include "nldb/SQL3Implementation.hpp"
#include "nldb/Utils/Enums.hpp"
#include "nldb/Utils/Variant.hpp"
#include "nldb/backends/sqlite3/DB/DB.hpp"
#include "nldb/nldb_json.hpp"

using namespace nldb;

int main(int argc, char* argv[]) {
    nldb::LogManager::Initialize();

    const auto printHelp = []() {
        std::cout << "\t usage: ./dump file.db 200  dump the first 200 items"
                     "\n\t usage: ./dump file.db all  dump all the items\n";
    };

    if (argc <= 1) {
        NLDB_ERROR("Database file expected as the first argument.");
        printHelp();
        return 1;
    } else if (argc == 1 && (strcmp(argv[1], "--help") == 0) ||
               strcmp(argv[1], "-help") == 0) {
        printHelp();
        return 1;
    } else if (argc <= 2) {
        NLDB_ERROR("Expecting limit as the second argument.");
        printHelp();
        return 1;
    }

    // optional, tweak the sqlite available memory.
    // With this example I reduced 70% of the memory usage.
    DBSL3 db(DBConfig {.page_size = 1000 * 16 * 2,
                       .page_cache_size = 1000 * 16,
                       .page_cache_N = 41});

    if (!db.open(argv[1])) {
        NLDB_ERROR("Could not open the database '{}' \n", argv[1]);
        db.throwLastError();
    }

    char* limitStr = argv[2];
    int limit = -1;
    try {
        limit = std::stoi(limitStr);
    } catch (...) {
    }

    Query<DBSL3> query(&db);
    const std::vector<Collection> collections = query.getCollections();

    NLDB_INFO("Dumping collection, limit {}: ", limitStr);
    for (auto& c : collections) {
        NLDB_INFO("\t- {}", c.getName());
        nldb::json res;
        if (strcmp(limitStr, "all") == 0 || limit < 0) {
            res = query.from(c.getName()).select().execute();
        } else {
            res = query.from(c.getName()).select().limit(limit).execute();
        }

        std::ofstream f(c.getName() + "_dump.json", std::ios_base::trunc);
        f << res;
        f.close();

        NLDB_INFO("\tFinished {0}, result was saved into {0}_dump.json",
                  c.getName());
    }

    nldb::LogManager::Shutdown();

    return 0;
}
