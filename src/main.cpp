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
#include "nldb/Utils/Variant.hpp"
#include "nldb/nldb_json.hpp"

using namespace nldb;

// void numbersExamples() {
//     DBSL3 db;
//     if (!db.open("./numbers.db")) {
//         std::cerr << "Could not open the database \n";
//         db.throwLastError();
//     }

//     auto numbersCollection = QueryFactory::create(&db, "numbers");

//     json numbers_json = {
//         {{"number_name", "pi"}, {"double_rep", M_PI}, {"integer_rep", 3}},

//         {{"number_name", "e"}, {"double_rep", M_E}, {"integer_rep", 2}},

//         {{"number_name", "log2(e)"},
//          {"double_rep", M_LOG2E},
//          {"integer_rep", 1}}};

//     numbersCollection.insert(numbers_json);

//     auto numbers = numbersCollection.select().execute();

//     std::cout << numbers << std::endl;
// }

int main() {
    nldb::LogManager::Initialize();

    auto md = Property(-1, "mds", PropertyType::STRING, -1);
    auto yy = Property(-1, "yy", PropertyType::STRING, -1);

    auto c = (md > yy) && (yy != md);

    auto c1 = md < yy;
    auto c2 = md < 200;

    auto c3 = c1 && c2;
    auto c4 = c1 || c3;
    auto c5 = ~c4;

    // numbersExamples();
    // return 0;

    DBSL3 db;

    if (!db.open("./tests.db")) {
        std::cerr << "Could not open the database \n";
        db.throwLastError();
    }

    auto collQuery = Query(&db);

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
        collQuery.collection("persona").get("id", "name", "contact"_obj);

    json result = collQuery.from("persona").select(id, name, contact).execute();

    std::cout << result;

    // auto [contact] =
    //     collQuery.collection("persona").collection("contact").only("address",
    //     "email");

    // auto [name, address, email] =
    //     collQuery.collection("persona").get("name", "contact.address",
    //     "contact.email");

    // auto cars = collQuery.collection("cars");

    // json data_cars = {
    //     {{"maker", "ford"}, {"model", "focus"}, {"year", 2011}},
    //     {{"maker", "ford"}, {"model", "focus"}, {"year", 2015}},
    //     {{"maker", "subaru"}, {"model", "impreza"}, {"year", 2003}}};

    // collQuery.from("cars").insert(data_cars);

    // auto [id, model, maker, year] = cars.get("id", "model", "maker",
    // "year");

    // coll

    // maybe auto [car_id, ...] = collection("cars").prepare(...)
    //       auto [race_winner, ...] = collection("races").prepare(...)
    //       query.select(car_id, c2, ..., cn, race_winner, ..., r2, ...,
    //       rn)
    //            .from("cars", "races")
    //            .where(car_id == race_winner).execute()

    // select id, model, maker and max year from each model
    // auto res1 = collQuery.from("cars")
    //                 .select(id, model, maker,
    //                 year.maxAs("year_newest_model")) .where(year > 1990)
    //                 .page(1, 10)
    //                 .groupBy(model, maker)
    //                 .execute();

    // std::cout << "\n\nRES1: " << res1 << std::endl << std::endl;

    // update first car, set year to 2100 and add a new property called
    // price int affected_update =
    //     collQuery.update(res1[0]["id"], {{"year", 2100}, {"price",
    //     50000}});

    // auto res2 = collQuery.select().page(1,
    // 10).sort(year.desc()).execute();

    // std::cout << "\n\nRES2: " << res2 << std::endl;

    // auto final = collQuery.select(model, maker, year).execute();

    // std::cout << "\n\nall before: " << final << std::endl;

    // int affected = collQuery.remove(1);

    // std::cout << "\n\naffected: " << affected << std::endl;

    // auto finalthen = collQuery.select(model, maker, year).execute();

    // std::cout << "\n\nall: " << finalthen << std::endl;

    nldb::LogManager::Shutdown();
}
