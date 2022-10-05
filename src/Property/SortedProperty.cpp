#include "nldb/Property/SortedProperty.hpp"

namespace nldb {
    SortedProperty::SortedProperty(Property pProp, SortType pType)
        : property(pProp), type(pType) {}
}  // namespace nldb