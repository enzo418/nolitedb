/**
 * this file shows how the program evaluates a string to obtain only the desired
 * properties.
 */

#include <iostream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

struct Prop {
    std::string name;
};

typedef std::variant<class Composed, Prop> Selectable;

struct Composed {
    Prop prop;
    std::vector<Selectable> subProps;
};

template <bool IsFirst = true>
Selectable readNextPropertyRecursive(auto& c, const auto& end) {
    std::string word;

    while (c != end) {
        if (*c == '{') {
            ++c;  // skip {
            Composed composed = {Prop {word}};

            // read its properties

            if (*c == ',' || *c == ']') {
                // errors could easily be improved
                throw std::runtime_error("expected property name at -> " +
                                         std::string(c, end));
            }

            do {
                if (*c == ',') c++;    // skip , to read the prop name
                while (*c <= 32) c++;  // skip spaces
                composed.subProps.push_back(
                    readNextPropertyRecursive<false>(c, end));
            } while (*c == ',');

            if (*c == '}') {  // composed ended
                c++;
                return composed;
            } else {
                throw std::runtime_error(
                    "Syntax error on composed property, expected '}' at the "
                    "end of the collection properties " +
                    word);
            }
        } else if (*c == '}' || *c == ',') {
            if constexpr (IsFirst) {
                throw std::runtime_error("Unexpected '" + std::string(*c, 1) +
                                         "' at the root property");
            }

            // found a '}' or a ',' outside a composed, that means that we are
            // just a property.
            return Prop {word};
        } else if (*c > 32) {
            word += *c++;
        } else {
            throw std::runtime_error("Syntax error on composed property near " +
                                     std::string(c, end));
        }
    }

    if constexpr (IsFirst) {
        if (word.length() != 0) {
            return Composed {Prop {word}};
        }

        throw std::runtime_error(
            "Unexpected end of string while reading a property");
    } else {
        return Prop {word};
    }
}

Composed readProperty(const std::string& str) {
    auto l_it = str.begin();  // create a l value iterator
    return std::get<Composed>(readNextPropertyRecursive(l_it, str.end()));
}

void print(const Prop& p, int tab = 0) {
    std::cout << std::string(tab, ' ') << p.name << std::endl;
}

void print(const Composed& p, int tab = 0) {
    std::cout << std::string(tab, ' ') << "- " << p.prop.name << std::endl;

    auto cb = [&tab](const auto& p) { print(p, tab + 2); };

    for (const auto& prop : p.subProps) {
        std::visit(cb, prop);
    }
}

void showInp() { std::cout << "\n input >"; }

int main() {
    std::string expr =
        "user{name, last_login_date, data{email, phone, address{city}}}";

    Composed res = readProperty(expr);

    showInp();
    std::cout << expr << "\n";
    print(res);

    std::string input;
    while (true) {
        showInp();
        std::cin >> input;
        Composed r = readProperty(input);
        print(r);
    }
}
