#include <iostream>
#include <iterator>
#include <string>
#include <tuple>
#include <variant>

#include "include/PropertyCondition.hpp"
#include "include/PropertyRep.hpp"
#include "include/Query.hpp"

int main() {
    auto factory = QueryFactory();

    auto [query, model, maker, year] =
        factory.properties("model", "maker", "year");

    std::cout << model.getName() << std::endl;

    auto ps = {model<year, model <= year, model> year, model >= year,
               model >= year};
    // auto res =
    //     q.where(model % "%ford%", year > 2000)
    //     .where(model = "focus")
    //     .execute();
}
