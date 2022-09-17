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

void numbersExamples() {
    DBSL3 db;
    if (!db.open("./numbers.db")) {
        std::cerr << "Could not open the database \n";
        db.throwLastError();
    }

    auto numbersCollection = QueryFactory::create(&db, "numbers");

    json numbers_json = {
        {{"number_name", "pi"}, {"double_rep", M_PI}, {"integer_rep", 3}},

        {{"number_name", "e"}, {"double_rep", M_E}, {"integer_rep", 2}},

        {{"number_name", "log2(e)"},
         {"double_rep", M_LOG2E},
         {"integer_rep", 1}}};

    numbersCollection.insert(numbers_json).execute();

    auto numbers = numbersCollection.select().execute();

    std::cout << numbers << std::endl;
}

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

    // numbersExamples();
    // return 0;

    DBSL3 db;

    if (!db.open("./tests.db")) {
        std::cerr << "Could not open the database \n";
        db.throwLastError();
    }

    auto collQuery = QueryFactory::create(&db, "cars");

    json cars = {{{"maker", "ford"}, {"model", "focus"}, {"year", 2011}},
                 {{"maker", "ford"}, {"model", "focus"}, {"year", 2015}},
                 {{"maker", "subaru"}, {"model", "impreza"}, {"year", 2003}}};

    collQuery.insert(cars).execute();

    auto [id, model, maker, year] =
        collQuery.prepareProperties("id", "model", "maker", "year");

    // maybe auto [car_id, ...] = collection("cars").prepare(...)
    //       auto [race_winner, ...] = collection("races").prepare(...)
    //       query.select(car_id, c2, ..., cn, race_winner, ..., r2, ..., rn)
    //            .from("cars", "races")
    //            .where(car_id == race_winner).execute()

    auto res1 =
        collQuery.select(id, model, maker, year.maxAs("year_newest_model"))
            .where(year > 1990)
            .page(1, 10)
            .groupBy(model, maker)
            .execute();

    std::cout << "\n\nRES1: " << res1 << std::endl << std::endl;

    auto res2 = collQuery.select()
                    .page(1, 10)
                    .groupBy(model, maker)
                    .sort(year.desc())
                    .execute();

    std::cout << "\n\nRES2: " << res2 << std::endl;

    auto final = collQuery.select(model, maker, year).execute();

    std::cout << "\n\nall before: " << final << std::endl;

    auto affected = collQuery.remove(1).execute();

    std::cout << "\n\naffected: " << affected << std::endl;

    auto finalthen = collQuery.select(model, maker, year).execute();

    std::cout << "\n\nall: " << finalthen << std::endl;
}
