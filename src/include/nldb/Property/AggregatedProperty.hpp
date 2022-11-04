#pragma once

#include "nldb/Property/Property.hpp"

namespace nldb {
    enum AggregationType { COUNT, AVG, SUM, MAX, MIN };

    struct AggregatedProperty {
        AggregatedProperty(Property pProp, AggregationType pType,
                           const char* pAlias)
            : type(pType), property(pProp), alias(pAlias) {}

        AggregationType type;
        Property property;
        const char* alias;
    };
}  // namespace nldb