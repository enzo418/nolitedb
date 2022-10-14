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

// void tweetsTest(DBSL3* db) {
//     std::ifstream f("twitter.json");
//     json data = json::parse(f);

//     auto collQuery = Query(db);

//     auto now = std::chrono::high_resolution_clock::now();
//     collQuery.from("tweets").insert(data);

//     std::cout << "took "
//               << (std::chrono::high_resolution_clock::now() - now) /
//                      std::chrono::milliseconds(1)
//               << " ms" << std::endl;
// }

void cars_example(Query<DBSL3>& collQuery) {
    auto cars = collQuery.collection("cars");

    json data_automaker = {{{"name", "ford"},
                            {"founded", "June 16, 1903"},
                            {"country", "United States"}},
                           // -------------
                           {{"name", "subaru"},
                            {"founded", "July 15, 1953"},
                            {"country", "Japan"}}};

    collQuery.from("automaker").insert(data_automaker);
    auto automakers = collQuery.collection("automaker");

    json data_cars = {
        {{"maker", "ford"}, {"model", "focus"}, {"year", 2011}},
        {{"maker", "ford"}, {"model", "focus"}, {"year", 2015}},
        {{"maker", "subaru"}, {"model", "impreza"}, {"year", 2003}}};

    collQuery.from("cars").insert(data_cars);

    auto [id, model, maker, year] = cars.get("id", "model", "maker", "year");

    auto automaker = automakers.group("id", "model", "maker", "year");

    // select id, model, maker and max year from each model
    auto res1 = collQuery.from("cars")
                    .select(id, model, maker, year.maxAs("year_newest_model"),
                            automaker)
                    .where(year > 1990 && automaker["name"] == maker)
                    .page(1, 10)
                    .groupBy(model, maker)
                    .execute();

    std::cout << "\n\nRES1: " << res1.dump(2) << std::endl << std::endl;

    // update first car, set year to 2100 and add a new property called
    // price
    collQuery.from("cars").update(res1[0]["id"],
                                  {{"year", 2100}, {"price", 50000}});

    auto res2 = collQuery.from("cars")
                    .select()
                    .page(1, 10)
                    .sortBy(year.desc())
                    .execute();

    std::cout << "\n\nRES2: " << res2.dump(2) << std::endl;

    auto final = collQuery.from("cars").select(model, maker, year).execute();

    std::cout << "\n\nall before: " << final << std::endl;

    collQuery.from("cars").remove(1);

    auto finalThen =
        collQuery.from("cars").select(model, maker, year).execute();

    std::cout << "\n\nall: " << finalThen.dump(2) << std::endl;
}

void example_notion_object(Query<DBSL3>& collQuery) {
    json data = json::array(
        {{// obj 1
          {"name", "carl"},
          {"information", {{"city", "hh"}, {"mail", "mail@at.com"}}}}});

    collQuery.from("persona").insert(data);
    // collQuery.from("persona").update(1, {{"name", "enzo a."}});
    // collQuery.from("persona").update(
    //     1, {{"contact", {{"email", "fake@fake.fake"}}}});

    // auto t = "contact"_obj;

    // collQuery.from("persona").insert({{"name", "enzo"}});

    auto [id, name, information] =
        collQuery.collection("persona").get("id", "name", "information"_obj);

    // auto cond = id > 2;

    // std::stringstream ot;
    // addWhereExpression(ot, cond);

    // std::cout << ot.str() << std::endl;

    // NLDB_ASSERT("id" == std::visit(getstr, cond.left), "is not id");

    json result = collQuery.from("persona")
                      .select(id, name, information)
                      .where(id != 9)
                      .execute();

    std::cout << result.dump(2) << std::endl;
}

int main() {
    nldb::LogManager::Initialize();

    auto md = Property(-1, "mds", PropertyType::STRING, -1);
    auto yy = Property(-1, "yy", PropertyType::STRING, -1);

    auto c = (md > yy) && (yy != md);

    auto c1 = md < 2;
    auto c2 = md < 200;

    auto c3 = c1 && c2;
    auto c4 = c1 || c3;
    auto c5 = ~c4;

    DBSL3 db;

    if (!db.open("./tests.db")) {
        std::cerr << "Could not open the database \n";
        db.throwLastError();
    }

    auto collQuery = Query(&db);

    // example_notion_object(collQuery);
    // return 0;

    json data = json::array({{// obj 1
                              {"name", "enzo"},
                              {"contact",
                               {{"phone", "12344"},
                                {"address", "fake st 99"},
                                {"email", "c.enzo@at.com"}}}

                             },  // end obj1
                             {
                                 // obj 2
                                 {"name", "pepe"},
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

    collQuery.from("persona").insert(data);
    // collQuery.from("persona").update(1, {{"name", "enzo a."}});
    // collQuery.from("persona").update(
    //     1, {{"contact", {{"email", "fake@fake.fake"}}}});

    // auto t = "contact"_obj;

    // collQuery.from("persona").insert({{"name", "enzo"}});

    auto [id, name, contact] =
        collQuery.collection("persona").get("id", "name", "contact{email}"_obj);

    // auto cond = id > 2;

    // std::stringstream ot;
    // addWhereExpression(ot, cond);

    // std::cout << ot.str() << std::endl;

    // NLDB_ASSERT("id" == std::visit(getstr, cond.left), "is not id");

    json result = collQuery.from("persona")
                      .select(id, name, contact)
                      .where(id != 9)
                      .sortBy(contact["email"].desc())
                      .execute();

    std::cout << result.dump(4);

    // auto [contact] =
    //     collQuery.collection("persona").collection("contact").only("address",
    //     "email");

    // auto [name, address, email] =
    //     collQuery.collection("persona").get("name", "contact.address",
    //     "contact.email");

    // cars_example(collQuery);

    nldb::LogManager::Shutdown();
}
