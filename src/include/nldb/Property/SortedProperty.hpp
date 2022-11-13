#pragma once

#include "nldb/Property/Property.hpp"

namespace nldb {
    enum SortType { ASC, DESC };

    struct SortedProperty {
        SortedProperty(Property pProp, SortType pType)
            : property(pProp), type(pType) {}
        Property property;
        SortType type;
    };
}  // namespace nldb