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
    Logger::setPrefixLevel(Logger::PrefixLevel::NONE);

    json ex3 = {
        {"happy", true},
        {"pi", 3.141},
        {"array", {1, 2, 3}},
        {"object", {{"a", 1}}},
    };

    std::cout << ex3 << std::endl;

    // even easier with structured bindings (C++17)
    for (auto& [key, value] : ex3.items()) {
        std::cout << key << " : " << value << " [" << value.type_name()
                  << "]\n";
    }

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
                 {{"maker", "subaru"}, {"model", "impreza"}, {"year", 2003}}};

    collQuery.insert(cars).execute();

    auto [model, maker, year] =
        collQuery.prepareProperties("model", "maker", "year");

    // maybe auto [car_id, ...] = collection("cars").prepare(...)
    //       auto [race_winner, ...] = collection("races").prepare(...)
    //       query.select(car_id, c2, ..., cn, race_winner, ..., r2, ..., rn)
    //            .from("cars", "races")
    //            .where(car_id == race_winner).execute()

    auto res = collQuery.select(model, maker, year)
                   .where(year > 2000 || model == "impreza")
                   .page(1, 10)
                   .execute();

    std::cout << "\n\nRES: " << res << std::endl;

    // std::cout << year.getName() << std::endl;

    // std::vector<PropertyCondition> ps = {model<year, model <= year, model>
    // year,
    //                                      model >= year, model >= year};

    // auto s = std::ref("asd");

    // auto p = model < "123123";

    // auto res =
    //      q.select(model, maker, year, /*count(*)*/)
    //      .where(model % "%ford%", year > 2000)
    //      .where(model = "focus")
    //      .execute();
}
