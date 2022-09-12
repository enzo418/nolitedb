cmake_minimum_required(VERSION 3.22)

project(noLiteDb)

file(GLOB_RECURSE SOURCES src/*.cpp)

# REMOVE MAIN so we can don't need to compile the code again for the tests
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

# Set cmake cxx flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -std=c++20")

# clangd:
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(src/include)
include_directories(vendor/sqlite)

# YAMLCPP
# add_subdirectory(vendor/yaml-cpp lib)
# set(YAML_CPP_INSTALL OFF)
# include_directories(vendor/yaml-cpp/include)

# --- SQLITE library
add_library(c_sqllite3 vendor/sqlite/sqlite/sqlite3.c)

# --- Compile library and exec

add_library(noLitedb_lb ${SOURCES})
#yaml-cpp
target_link_libraries(noLitedb_lb c_sqllite3)

add_executable(noLiteDb src/main.cpp)
target_link_libraries(noLiteDb noLitedb_lb)

# --- TESTS
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
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(tests)
