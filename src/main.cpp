#include <functional>
#include <iostream>
#include <iterator>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

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

    auto model = PropertyRep("model");
    auto year = PropertyRep("year");

    auto c = (model > year) && (year != model);

    auto c1 = model < year;
    auto c2 = model < 200;

    auto c3 = c1 && c2;
    auto c4 = c1 || c3;
    auto c5 = ~c4;

    std::cout << "Result: " << c4.getStatement() << std::endl;

    DBSL3 db;

    if (!db.open("./tests.db")) {
        std::cerr << "Could not open the database \n";
        db.throwLastError();
    }

    // auto id = db.executeAndGetFirstInt(
    //     "SELECT id FROM property where coll_id = @colid and name = @name",
    //     {{"@colid", 1}, {"@name", "maker"}});

    // std::cout << "id: " << id.value_or(-1) << std::endl;

    // return 0;

    auto factory = QueryFactory();
    auto collQuery = factory.create(&db, "cars");

    json cars = {{{"maker", "ford"}, {"model", "focus"}, {"year", 2011}},
                 {{"maker", "subaru"}, {"model", "impreza"}, {"year", 2003}}};

    collQuery.insert(cars);

    // auto [query, model, maker, year] =
    //     factory.properties("model", "maker", "year");

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
