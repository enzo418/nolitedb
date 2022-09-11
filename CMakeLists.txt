cmake_minimum_required(VERSION 3.22)

project(katdb)

file(GLOB SOURCES src/*.cpp)
 
# REMOVE MAIN so we can don't need to compile the code again for the tests
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

# Set cmake cxx flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -std=c++20")

# clangd:
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(src/include)

add_library(katdb_lib ${SOURCES})

add_executable(katdb src/main.cpp)

target_link_libraries(katdb katdb_lib)

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
  katdb_lib
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(tests)
