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

    // remove("./tests.db");
    if (!db.open("./tests.db" /*":memory:"*/)) {
        std::cerr << "Could not open the database \n";
        db.throwLastError();
    }

    auto query = Query(&db);

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
    query.from("persona").insert(data);

    std::cout << "insert took "
              << (std::chrono::high_resolution_clock::now() - now) /
                     std::chrono::milliseconds(1)
              << " ms" << std::endl;

    auto [id, name, aliases, contact] = query.collection("persona").get(
        "_id", "name", "aliases", "contact{email}"_obj);

    // TODO: Suppress fields with !<field>
    auto [_id, _contact] = query.collection("persona").get(
        "_id", "contact{_id, email, location{_id}}"_obj);

    now = std::chrono::high_resolution_clock::now();
    auto result = query.from("persona")
                      .select()
                      //   .where(_id != 9)
                      //   .sortBy(contact["email"].desc())
                      .execute();

    std::cout << "select took "
              << (std::chrono::high_resolution_clock::now() - now) /
                     std::chrono::milliseconds(1)
              << " ms" << std::endl;

    std::cout << result.dump(4);

    /* ------------------ filter out fields ----------------- */
    // you can pass (!<field>)* to the group query and it will get all the
    // fields but <field>. We don't select any fields by default, so if you only
    // want the person name you just .get("name") and then .select(name).

    // get all persons without id and name.
    Object allButIdName = query.collection("persona").group("!_id", "!name");

    result = query.from("persona")
                 .select(allButIdName)
                 //  you can use all the person members in where/sort/groupBy,
                 //  even if you didn't select them.
                 .where(allButIdName["name"] != "test")
                 .execute();

    std::cout << "all but _id, name: " << result.dump(2) << std::endl;

    /* ------- suppress fields in embedded documents ------ */
    // suppose we want to suppress all the ids from the inner documents and the
    // phone from contact
    Object allButIds = query.collection("persona").group(
        "!_id", "contact{!_id, !phone, location{!_id}}"_obj);

    result = query.from("persona")
                 .select(allButIds)
                 .where(allButIds["contact.phone"] != "test" &&
                        allButIds["name"] != "hola")
                 .sortBy(allButIds["contact.location._id"].asc())
                 .execute();

    std::cout << "all but _id and contact phone " << result.dump(2)
              << std::endl;

    nldb::LogManager::Shutdown();
}
