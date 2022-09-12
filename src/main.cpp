#include <functional>
#include <iostream>
#include <iterator>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

#include "dbwrapper/sq3wrapper/DB.hpp"
#include "include/PropertyRep.hpp"
#include "include/Query.hpp"

int main() {
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

    if (!db.open(":memory:")) {
        std::cerr << "Could not open the database \n";
        db.throwLastError();
    }

    auto factory = QueryFactory();
    auto collQuery = factory.create(db, "cars");

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
