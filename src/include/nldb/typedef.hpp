#pragma once

#include <cstdint>

typedef int_least64_t snowflake;

// is not inserted into the database, it is used to avoid the use of null
// pointers.
constexpr snowflake NullID = -1;