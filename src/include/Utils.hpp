#pragma once

#include <set>
#include <string>
#include <vector>

namespace utils {
    void replaceAllOccurrences(std::string& str, const std::string& from,
                               const std::string& to);

    template <typename T>
    inline void concat(std::vector<T>& dst, const std::vector<T>& src) {
        dst.insert(dst.end(), src.begin(), src.end());
    }

    template <typename T>
    void inline concat(std::set<T>& dst, const std::set<T>& src) {
        dst.insert(src.begin(), src.end());
    }
}  // namespace utils