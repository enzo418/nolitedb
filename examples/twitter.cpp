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

    DBSL3 db;

    remove("./twitter.db");

    if (!db.open("./twitter.db" /*":memory:"*/)) {
        std::cerr << "Could not open the database \n";
        db.throwLastError();
    }

    std::ifstream f("twitter.json");
    json data = json::parse(f)["statuses"];

    auto query = Query(&db);

    auto now = std::chrono::high_resolution_clock::now();
    query.from("tweets").insert(data);

    std::cout << "took "
              << (std::chrono::high_resolution_clock::now() - now) /
                     std::chrono::milliseconds(1)
              << " ms" << std::endl;

    now = std::chrono::high_resolution_clock::now();
    // Select all won't work because this dataset has more than 64 properties,
    // which is the maximum join limit of sqlite. If you really want to handle
    // data of this size then you can use something like firebird that enables
    // up to 256 joins.
    // https://www.ibphoenix.com/resources/documents/general/doc_323#:~:text=Maximum%20number%20of%20joined%20tables,evaluations%20required%20by%20the%20joins.
    auto res = query.from("tweets").select().page(1, 1000).execute();

    std::cout << res << std::endl;

    std::cout << "size: " << res.size() << std::endl;

    std::cout << "select took "
              << (std::chrono::high_resolution_clock::now() - now) /
                     std::chrono::milliseconds(1)
              << " ms" << std::endl;

    nldb::LogManager::Shutdown();
}
