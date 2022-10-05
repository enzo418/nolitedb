#pragma once

namespace nldb {
    // from https://en.cppreference.com/w/cpp/utility/variant/visit
    // ------ >= c++17
    // helper type for the visitor #4
    template <class... Ts>
    struct overloaded : Ts... {
        using Ts::operator()...;
    };
    // explicit deduction guide (not needed as of C++20)
    template <class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;
}  // namespace nldb