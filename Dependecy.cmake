include(ExternalProject)

set(DEPENDENCY_INSTALL_DIR ${PROJECT_BINARY_DIR}/install)
set(DEPENDENCY_INCLUDE_DIR ${DEPENDENCY_INSTALL_DIR}/include)
set(DEPENDENCY_LIB_DIR ${DEPENDENCY_INSTALL_DIR}/lib)

function(add_dep dep)
    set(DEPENDENCY_LIST ${DEPENDENCY_LIST} ${dep} PARENT_SCOPE)
endfunction()

function(add_include include_dir)
    set(DEPENDENCY_INCLUDE_LIST ${DEPENDENCY_INCLUDE_LIST} ${include_dir} PARENT_SCOPE)
endfunction()

function(add_lib lib)
    set(DEPENDENCY_LIBS ${DEPENDENCY_LIBS} ${lib} PARENT_SCOPE)
endfunction(add_lib)

# ----------------------- SPDLOG ----------------------- #
ExternalProject_Add(
    dep-spdlog
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${DEPENDENCY_INSTALL_DIR}
    TEST_COMMAND ""
)

add_dep(dep-spdlog)
add_include(${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include)
add_lib(spdlog)

# ----------------------- SQLITE3 ---------------------- #
add_library(xsqllite3 ${CMAKE_CURRENT_SOURCE_DIR}/vendor/sqlite/sqlite/sqlite3.c)

add_include(${CMAKE_CURRENT_SOURCE_DIR}/vendor/sqlite)
add_lib(xsqllite3)

# -------------------- NLOHMANN JSON ------------------- #
ExternalProject_Add(
    dep-json
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/json
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${DEPENDENCY_INSTALL_DIR}
        -DNLOHMANN_JSON_INCLUDE_INSTALL_DIR=${DEPENDENCY_INSTALL_DIR}
        -DJSON_Install=ON
        -DJSON_BuildTests=OFF
    TEST_COMMAND ""
)

add_dep(dep-json)
add_include(${CMAKE_CURRENT_SOURCE_DIR}/vendor/json/include)
# add_lib(nlohmann_json) # dependecy adds the .Targets file and handles the link_library

# --------------------- MAGIC ENUM --------------------- #
add_include(${CMAKE_CURRENT_SOURCE_DIR}/vendor/magic_enum/include)

# ---------------------- LRU CACHE --------------------- #
add_include(${CMAKE_CURRENT_SOURCE_DIR}/vendor/lrucache11/include)

# ----------------------- OTHERS ----------------------- #
add_include(${CMAKE_CURRENT_SOURCE_DIR}/vendor/others)