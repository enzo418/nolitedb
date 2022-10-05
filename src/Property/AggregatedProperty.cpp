#include "nldb/Property/AggregatedProperty.hpp"

namespace nldb {
    AggregatedProperty::AggregatedProperty(Property pProp,
                                           AggregationType pType,
                                           const char* pAlias)
        : property(pProp), type(pType), alias(pAlias) {}
}  // namespace nldb