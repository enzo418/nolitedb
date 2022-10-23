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

using namespace nldb;

int main() {
    nldb::LogManager::Initialize();

    DBSL3 db;

    if (!db.open("./tests.db" /*":memory:"*/)) {
        std::cerr << "Could not open the database \n";
        db.throwLastError();
    }

    auto collQuery = Query(&db);

    json data = json::array({{// obj 1
                              {"name", "enzo"},
                              {"aliases", {"x", "p", "r"}},
                              {"contact",
                               {{"phone", "12344"},
                                {"address", "fake st 99"},
                                {"email", "c.enzo@at.com"}}}

                             },  // end obj1
                             {
                                 // obj 2
                                 {"name", "pepe"},
                                 {"aliases", {"pepe", "mr pepe"}},
                                 {
                                     "contact",
                                     {// contact
                                      {"phone", "999"},
                                      {"location",
                                       {{"city", "big city"},
                                        {"address", "not a fake st 89"},
                                        {"country", "argentina"}}},
                                      {"email", "f@f.f"}}  // contact
                                 }
                                 //
                             }});

    auto now = std::chrono::high_resolution_clock::now();
    collQuery.from("persona").insert(data);

    std::cout << "insert took "
              << (std::chrono::high_resolution_clock::now() - now) /
                     std::chrono::milliseconds(1)
              << " ms" << std::endl;

    auto [id, name, aliases, contact] = collQuery.collection("persona").get(
        "_id", "name", "aliases", "contact{email}"_obj);

    auto [_id, _contact] = collQuery.collection("persona").get(
        "_id", "contact{_id, email, location{_id}}"_obj);

    now = std::chrono::high_resolution_clock::now();
    auto result = collQuery.from("persona")
                      .select()
                      .where(_id != 9)
                      .sortBy(contact["email"].desc())
                      .execute();

    std::cout << "select took "
              << (std::chrono::high_resolution_clock::now() - now) /
                     std::chrono::milliseconds(1)
              << " ms" << std::endl;

    std::cout << result.dump(4);

    nldb::LogManager::Shutdown();
}
