cmake_minimum_required(VERSION 3.22)

project(noLiteDb)

option(BUILD_EXAMPLES "Build examples" OFF)

# -------------------- CONFIGURATION ------------------- #

option(NLDB_SHOW_ID_WARNING "Prints a warning if the requested is not the internal identifier." ON)

# ------------------ CONFIGURATION DEV ----------------- #
option(NLDB_DEBUG_CACHE_BUFFER "Shows cache hit/miss and buffer warnings" OFF)

option(NLDB_DEBUG_QUERY "Displays executed queries and human-readable aliases" OFF)

option(NLDB_PROFILE "Profile query execution time." OFF)

option(NLDB_SANITIZE_EXAMPLES "Use the compiler sanitizer in the examples." OFF)

# ----------------------- SOURCES ---------------------- #
file(GLOB_RECURSE SOURCES src/*.cpp)

# Set std version 
# Note: I do it this way because otherwise export compile commands doesn't export it
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

# clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE )
  # Compilation type
  set(CMAKE_BUILD_TYPE DEBUG)
endif()

include(Dependecy.cmake)

message(STATUS "NLDB dependencies: ${DEPENDENCY_LIST}")
message(STATUS "NLDB dependency libs: ${DEPENDENCY_LIBS}")
message(STATUS "NLDB dependency lib includes: ${DEPENDENCY_INCLUDE_DIR}")

include_directories(src/include)
include_directories(src)

# --------------------- NOLITE --------------------- #
add_library(noLitedb_lb ${SOURCES})

add_dependencies(noLitedb_lb ${DEPENDENCY_LIST})
target_include_directories(noLitedb_lb PUBLIC ${DEPENDENCY_INCLUDE_DIR} ${DEPENDENCY_INCLUDE_LIST})

target_compile_options(noLitedb_lb PRIVATE -Wall -Wextra -Wpedantic -Werror)

# optional flags, e.g. -DNLDB_LIB_OPTIONAL_FLAGS:STRING="-w"
target_compile_options(noLitedb_lb PRIVATE "${NLDB_LIB_OPTIONAL_FLAGS}")

# in c++20 the expression Property == Property warns about something we don't care.
add_compile_options(-Wno-ambiguous-reversed-operator)

# boolean definitions
foreach(_VAR IN ITEMS
    NLDB_DEBUG_QUERY
    NLDB_DEBUG_CACHE_BUFFER
    NLDB_SHOW_ID_WARNING
    NLDB_PROFILE
)
    if(${_VAR})
        target_compile_definitions(noLitedb_lb PUBLIC "${_VAR}")
    endif()
endforeach()

target_link_libraries(noLitedb_lb PUBLIC ${DEPENDENCY_LIBS})
target_link_directories(noLitedb_lb PUBLIC ${DEPENDENCY_LIB_DIR})

# ------------------ NOLITE EXECUTABLE ----------------- #
if (NLDB_SANITIZE_EXAMPLES)
  set(NLDB_SANITIZER_ARGS -fsanitize=address -fno-omit-frame-pointer)
  message(STATUS "Using sanitizer options: ${NLDB_SANITIZER_ARGS}")
endif()

if(BUILD_EXAMPLES OR PROJECT_IS_TOP_LEVEL)
    file(GLOB nldb_example_files 
        "examples/*.cpp")

    foreach(filename ${nldb_example_files})
        STRING(REGEX REPLACE
        "([/\\][A-Za-z0-9_]+)+[/\\]([A-Za-z0-9_]+)\.c(pp)?"
        "\\2" nldb_exec_name ${filename}) # get exec output name
    
        add_executable(${nldb_exec_name} ${filename})
        target_link_libraries(${nldb_exec_name} noLitedb_lb "-lz")
        
        if(NLDB_SANITIZE_EXAMPLES)
          target_compile_options(${nldb_exec_name} PRIVATE ${NLDB_SANITIZER_ARGS})
          target_link_options(${nldb_exec_name} PRIVATE ${NLDB_SANITIZER_ARGS})
        endif()
    endforeach()
endif()

# ------------------------ TESTS ----------------------- #
add_subdirectory(vendor/googletest)
include_directories(vendor/googletest/googletest/include)

enable_testing()

file(GLOB_RECURSE TESTS_SRC tests/*.cpp)

add_executable(
  tests
  ${TESTS_SRC}
)

target_link_libraries(
  tests
  noLitedb_lb
  GTest::gtest
)

if (CMAKE_BUILD_TYPE MATCHES "DEBUG")
  add_definitions( -DLOGGER_MAX_LOG_LEVEL_PRINTED=6 ) # trace
else ()
  add_definitions( -DLOGGER_MAX_LOG_LEVEL_PRINTED=1 ) # error
endif()

include(GoogleTest)
gtest_discover_tests(tests)
