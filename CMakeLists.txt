cmake_minimum_required(VERSION 3.22)

project(noLiteDb)

# -------------------- CONFIGURATION ------------------- #

# A string used to differentiate between a user id and a document id.
set(NLDB_INTERNAL_ID "_id")

option(NLDB_SHOW_ID_WARNING "Prints a warning if the requested is not the internal identifier." ON)

# ------------------ CONFIGURATION DEV ----------------- #
option(NLDB_DEBUG_CACHE_BUFFER "Shows cache hit/miss and buffer warnings" OFF)


# ----------------------- SOURCES ---------------------- #
file(GLOB_RECURSE SOURCES src/*.cpp)

# REMOVE MAIN from sources
# Note: this way we don't need to compile the code again for the tests
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

# Set std version 
# Note: I do it this way because otherwise export compile commands doesn't export it
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++20")

# clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compilation type
set(CMAKE_BUILD_TYPE DEBUG)

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
target_compile_definitions(noLitedb_lb 
    PUBLIC 
    "NLDB_INTERNAL_ID=\"${NLDB_INTERNAL_ID}\"" 
    NLDB_SHOW_ID_WARNING=${NLDB_SHOW_ID_WARNING}
    NLDB_DEBUG_CACHE_BUFFER=${NLDB_DEBUG_CACHE_BUFFER}
)

target_link_libraries(noLitedb_lb PUBLIC ${DEPENDENCY_LIBS})
target_link_directories(noLitedb_lb PUBLIC ${DEPENDENCY_LIB_DIR})

# ------------------ NOLITE EXECUTABLE ----------------- #
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} noLitedb_lb)

# ------------------------ TESTS ----------------------- #
add_subdirectory(vendor/googletest)
include_directories(vendor/googletest/googletest/include)

enable_testing()

file(GLOB TESTS_SRC tests/*.cpp)

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
  add_definitions( -D LOGGER_MAX_LOG_LEVEL_PRINTED=6 ) # trace
else ()
  add_definitions( -D LOGGER_MAX_LOG_LEVEL_PRINTED=1 ) # error
endif()

include(GoogleTest)
gtest_discover_tests(tests)
