#pragma once

#include "nldb/Property/Property.hpp"

namespace nldb {
    enum SortType { ASC, DESC };

    struct SortedProperty {
        SortedProperty(Property, SortType);
        SortType type;
        Property property;
    };
}  // namespace nldb