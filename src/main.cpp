#include <functional>
#include <iostream>
#include <iterator>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

#include "include/PropertyCondition.hpp"
#include "include/PropertyRep.hpp"
#include "include/Query.hpp"

int main() {
    auto factory = QueryFactory();

    auto [query, model, maker, year] =
        factory.properties("model", "maker", "year");

    std::cout << year.getName() << std::endl;

    std::vector<PropertyCondition> ps = {model<year, model <= year, model> year,
                                         model >= year, model >= year};

    auto s = std::ref("asd");

    auto p = model < "123123";

    // auto res =
    //      q.select(model, maker, year, /*count(*)*/)
    //      .where(model % "%ford%", year > 2000)
    //      .where(model = "focus")
    //      .execute();
}
