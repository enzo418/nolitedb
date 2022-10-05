#pragma once

#include "nldb/Property/Property.hpp"

namespace nldb {
    enum AggregationType { COUNT, AVG, SUM, MAX, MIN };

    struct AggregatedProperty {
        AggregatedProperty(Property, AggregationType, const char* alias);
        AggregationType type;
        Property property;
        const char* alias;
    };
}  // namespace nldb