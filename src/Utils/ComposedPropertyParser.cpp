#include "nldb/Utils/ComposedPropertyParser.hpp"

namespace nldb::utils {
    ComposedProperty readComposedProperty(const std::string& str, int collID,
                                          Repositories* repos) {
        auto l_it = str.begin();  // create a l value iterator
        // we are sure it's a ComposedProperty since is the first iteration.
        return std::get<ComposedProperty>(
            readNextPropertyRecursive(l_it, str.end(), collID, repos));
    }
}  // namespace nldb::utils