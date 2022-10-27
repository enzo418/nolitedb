#include <array>
#include <cstddef>
#include <exception>
#include <iostream>

/**
 * @brief Checks if a string follows the object syntax.
 *
 * @tparam IsFirst is first call to this function?
 * @tparam N char array length
 * @param start from where to start the test
 * @return constexpr std::array<size_t, 2> first element is a boolean wether the
 * read string was valid or not, last element is the last position checked
 */
template <bool IsFirst, size_t N>
constexpr std::array<size_t, 2> isValidObject(const char (&exp)[N],
                                              size_t start = 0) {
    size_t i = start;
    char c = exp[i];
    bool foundFirstCurlyOpen = false;

    while (i < N) {
        c = exp[i];

        if (c == '{') {
            foundFirstCurlyOpen = true;
            ++i;  // skip {

            if (exp[i] == ',' || exp[i] == '}') {
                return {false, i};
            }

            do {
                if (exp[i] == ',') i++;    // skip , to read the prop name
                while (exp[i] <= 32) i++;  // skip spaces

                auto result = isValidObject<false>(exp, i);
                if (!result[0]) {
                    return {false, i};
                } else {
                    i = result[1];
                }

            } while (exp[i] == ',');

            if (exp[i] == '}') {  // composed ended
                i++;
                return {true, i};
            } else {
                return {false, i};
            }
        } else if (c == '}' || c == ',') {
            if constexpr (IsFirst) {
                // Unexpected character at the root property
                return {false, i};
            }

            // found a } or a , outside a composed, that means that
            // we are just a property.
            return {true, i};
        } else if (c > 32) {
            i++;
        } else {
            return {false, i};
        }
    }

    if constexpr (IsFirst) {
        return {i > 0, i};
    } else {
        return {true, i};
    }
}

template <size_t N>
constexpr bool IsProperty(const char (&exp)[N]) {
    size_t i = 0;
    while (i < N - 1) {
      char c = exp[i];
      // Match \w (character a-z, A-Z, 0-9, including _ (underscore))
      // I know that the json standar allows almost anything as a the member
      // key, but mostly you will use an \w identifier. If you still would
      // want to use other caracters, you need to use the _prop suffix.

      // https://www.rfc-editor.org/rfc/rfc7159#section-1

      if (!(/*A-Z*/ (c >= 65 && c <= 90) || /*_*/ (c == 95) ||
            /*a-z*/ (c >= 97 && c <= 122) ||
            /*0-9*/ (c >= 48 && c <= 57))) {
        return false;
      }

      i++;
    }

    return true;
}

template <size_t N>
constexpr bool IsObject(const char (&exp)[N]) {
    return isValidObject<true>(exp)[0];
}

int main() {
  // -- Object

  // CORRECT SYNTAX
  static_assert(IsObject("user{a}"), "expected an object");

  static_assert(IsObject("user{details{gps_position, city{name, state}}}"),
                "expected an object");

  // WRONG SYNTAX
  static_assert(!IsObject("usera}"), "wasn't expecting an object");

  static_assert(!IsObject("wrongComma{missing comma}"),
                "wasn't expecting an object");

  static_assert(!IsObject("wrongComma{not, missing, comma, but{curly}"),
                "wasn't expecting an object");

  // Debug:
  // std::cout << "Is object? " << IsObject("user{a}") << std::endl;

  // -- Property
  static_assert(IsProperty("name"), "Expected a property");
  static_assert(IsProperty("_na123123me44"), "Expected a property");
  static_assert(IsProperty("33312_3me44__"), "Expected a property");

  static_assert(!IsProperty("na$me"), "wasn't expecting a property");
  static_assert(!IsProperty("na\"me"), "wasn't expecting a property");
  //   std::cout << "Is prop? " << IsProperty("name") << std::endl;
}