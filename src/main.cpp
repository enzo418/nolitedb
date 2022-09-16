#include <functional>
#include <iostream>
#include <iterator>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

#include "Enums.hpp"
#include "dbwrapper/sq3wrapper/DB.hpp"
#include "dbwrapper/sq3wrapper/DBInitializer.hpp"
#include "include/PropertyRep.hpp"
#include "include/Query.hpp"
#include "logger/Logger.h"
#include "nlohmann/json.hpp"

using namespace nlohmann;

int main() {
    auto md = PropertyRep("md", -1, PropertyType::STRING);
    auto yy = PropertyRep("yy", -1, PropertyType::STRING);

    auto c = (md > yy) && (yy != md);

    auto c1 = md < yy;
    auto c2 = md < 200;

    auto c3 = c1 && c2;
    auto c4 = c1 || c3;
    auto c5 = ~c4;

    std::cout << "Result: " << c4.getStatement() << std::endl;

    DBSL3 db;

    if (!db.open("./tests.db")) {
        std::cerr << "Could not open the database \n";
        db.throwLastError();
    }

    auto factory = QueryFactory();
    auto collQuery = factory.create(&db, "cars");

    json cars = {{{"maker", "ford"}, {"model", "focus"}, {"year", 2011}},
                 {{"maker", "ford"}, {"model", "focus"}, {"year", 2015}},
                 {{"maker", "subaru"}, {"model", "impreza"}, {"year", 2003}}};

    collQuery.insert(cars).execute();

    auto [model, maker, year] =
        collQuery.prepareProperties("model", "maker", "year");

    // maybe auto [car_id, ...] = collection("cars").prepare(...)
    //       auto [race_winner, ...] = collection("races").prepare(...)
    //       query.select(car_id, c2, ..., cn, race_winner, ..., r2, ..., rn)
    //            .from("cars", "races")
    //            .where(car_id == race_winner).execute()

    auto res = collQuery.select(model, maker, year.maxAs("year_newest_model"))
                   .where(year > 1990)
                   .page(1, 10)
                   .groupBy(model, maker)
                   //    .sort(Query::Asc(model), maker.COUNT())
                   .execute();

    std::cout << "\n\nRES: " << res << std::endl;
}
