cmake_minimum_required(VERSION 3.22)

project(noLiteDb)

option(BUILD_EXAMPLES "Build examples" OFF)

option(NLDB_BUILD_SHARED "Build shared library" OFF)

# -------------------- CONFIGURATION ------------------- #

# A string used to differentiate between a user id and a document id.
set(NLDB_INTERNAL_ID "_id" CACHE STRING "string to use internaly as the id")

option(NLDB_SHOW_ID_WARNING "Prints a warning if the requested is not the internal identifier." ON)

option(NLDB_LOGGING "Enable/Disable loggin" ON)

# TODO: maybe make this a database level option
option(NLDB_ENABLE_DOUBLE_DOWNCASTING "Enable conversion from double to int with possible data loss. This happens when you try to set a double value into a integer property. Else it will throw an error." OFF)

# ------------------ CONFIGURATION DEV ----------------- #
option(NLDB_DEBUG_CACHE_BUFFER "Shows cache hit/miss and buffer warnings" OFF)

option(NLDB_DEBUG_QUERY "Displays executed queries and human-readable aliases" OFF)

option(NLDB_PROFILE "Profile query execution time." OFF)

option(NLDB_SANITIZE_EXAMPLES "Use the compiler sanitizer in the examples." OFF)

# disabled by default, compilers change warnings over time
option(WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

# ----------------------- SOURCES ---------------------- #
file(GLOB_RECURSE SOURCES src/*.cpp)

# Set std version 
# Note: I do it this way because otherwise export compile commands doesn't export it
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

# clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# CMAKE_BUILD_TYPE = RELEASE | DEBUG <default>
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE DEBUG)
endif()

include(Dependecy.cmake)

message(STATUS "NLDB dependencies: ${DEPENDENCY_LIST}")
message(STATUS "NLDB dependency libs: ${DEPENDENCY_LIBS}")
message(STATUS "NLDB dependency lib includes: ${DEPENDENCY_INCLUDE_DIR}")

add_include(${CMAKE_CURRENT_SOURCE_DIR}/src/include)

# --------------------- NOLITE --------------------- #
if(NLDB_BUILD_SHARED OR BUILD_SHARED_LIBS)
  set_property(TARGET xsqllite3 PROPERTY POSITION_INDEPENDENT_CODE ON)

  add_library(nolitedb SHARED ${SOURCES})
else()
  add_library(nolitedb STATIC ${SOURCES})
endif()

add_dependencies(nolitedb ${DEPENDENCY_LIST})
target_include_directories(nolitedb PUBLIC ${DEPENDENCY_INCLUDE_DIR} ${DEPENDENCY_INCLUDE_LIST})

target_compile_options(nolitedb PRIVATE -Wall -Wextra -Wpedantic)

if(WARNINGS_AS_ERRORS)
  target_compile_options(nolitedb PRIVATE -Werror)
endif()

# optional flags, e.g. -DNLDB_LIB_OPTIONAL_FLAGS:STRING="-w"
target_compile_options(nolitedb PRIVATE "${NLDB_LIB_OPTIONAL_FLAGS}")

# in c++20 the expression Property == Property warns about something we don't care.
add_compile_options(-Wno-ambiguous-reversed-operator)

# boolean definitions
foreach(_VAR IN ITEMS
    NLDB_DEBUG_QUERY
    NLDB_DEBUG_CACHE_BUFFER
    NLDB_SHOW_ID_WARNING
    NLDB_PROFILE
    NLDB_INSTALL
    NLDB_LOGGING
    NLDB_ENABLE_DOUBLE_DOWNCASTING
)
    if(${_VAR})
        target_compile_definitions(nolitedb PUBLIC "${_VAR}")
    endif()
endforeach()

# valued definitions
target_compile_definitions(nolitedb 
    PUBLIC 
    "NLDB_INTERNAL_ID=\"${NLDB_INTERNAL_ID}\"" 
)

target_link_libraries(nolitedb PUBLIC xsqllite3)

if(NLDB_LOGGING)
  target_link_libraries(nolitedb PUBLIC spdlog)
endif()

target_link_directories(nolitedb PUBLIC ${DEPENDENCY_LIB_DIR})

# ------------------ NOLITE EXECUTABLE ----------------- #
if (NLDB_SANITIZE_EXAMPLES)
  set(NLDB_SANITIZER_ARGS -fsanitize=address -fno-omit-frame-pointer)
  message(STATUS "Using sanitizer options: ${NLDB_SANITIZER_ARGS}")
endif()

if(BUILD_EXAMPLES)
    file(GLOB nldb_example_files 
        "examples/*.cpp")

    foreach(filename ${nldb_example_files})
        STRING(REGEX REPLACE
        "([/\\][A-Za-z0-9_]+)+[/\\]([A-Za-z0-9_]+)\.c(pp)?"
        "\\2" nldb_exec_name ${filename}) # get exec output name
    
        add_executable(${nldb_exec_name} ${filename})
        target_link_libraries(${nldb_exec_name} nolitedb "-lz")
        
        if(NLDB_SANITIZE_EXAMPLES)
          target_compile_options(${nldb_exec_name} PRIVATE ${NLDB_SANITIZER_ARGS})
          target_link_options(${nldb_exec_name} PRIVATE ${NLDB_SANITIZER_ARGS})
        endif()
    endforeach()
endif()

# ------------------------ TESTS ----------------------- #
if (NLDB_TEST)
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
    nolitedb
    GTest::gtest
  )

  include(GoogleTest)
  gtest_discover_tests(tests)
endif()

# ------------------------ INSTALL ----------------------- #
include(GNUInstallDirs)
set(cmake_package_name nolitedb CACHE INTERNAL "")

# Create the CMake package file descriptors.
if (NLDB_INSTALL)  
  install(TARGETS nolitedb xsqllite3
          LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
          ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
          RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

  # copy the vendors includes
  foreach(vendor_include_dir ${DEPENDENCY_INCLUDE_LIST})
    install(DIRECTORY "${vendor_include_dir}" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
  endforeach()

  configure_file("${CMAKE_CURRENT_LIST_DIR}/nldb_config.h.in" "${CMAKE_INSTALL_PREFIX}/tmp/nldb_config.h")
  install(FILES "${CMAKE_INSTALL_PREFIX}/tmp/nldb_config.h" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/nldb/")
  
  # TODO: pc, pkg, config, version ...
endif()