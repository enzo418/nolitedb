#include "nldb/Utils/String.hpp"

namespace utils {
    void replaceAllOccurrences(std::string& str, const std::string& from,
                               const std::string& to) {
        if (from.empty()) return;

        size_t pos;
        size_t offset = 0;
        const size_t fromSize = from.size();
        const size_t increment = to.size();
        while (std::string::npos != (pos = str.find(from, offset))) {
            str.replace(pos, fromSize, to);
            offset = pos + increment;
        }
    }
}  // namespace utils