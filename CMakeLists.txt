cmake_minimum_required(VERSION 3.20)

project(recomp_mod_native_lib)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TARGET_NAME $ENV{LIB_NAME})

include_directories("./offline_build")
include_directories("./include/extlib")
include_directories("./include/mod")
include_directories("./include/dummy_headers")
include_directories("./src/extlib")
include_directories("./src/extlib/database")
include_directories("./lib")
include_directories(".")


add_library(${TARGET_NAME} SHARED)

add_subdirectory("./src/extlib")

set_target_properties(${TARGET_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "./arc/"
    LIBRARY_OUTPUT_DIRECTORY "./lib/"
    RUNTIME_OUTPUT_DIRECTORY "./lib/"
    PREFIX ""
)

add_subdirectory("./lib/zlib/")
include_directories("./lib/zlib/")
target_link_libraries(${TARGET_NAME} PRIVATE zlibstatic)

set(ZLIB_INCLUDE_DIR "./lib/zlib/")
set(ZLIB_LIBRARY "./lib/zlib/")

set(PNG_SHARED OFF CACHE BOOL "" FORCE)
set(PNG_STATIC ON CACHE BOOL "" FORCE)
set(PNG_TESTS OFF CACHE BOOL "" FORCE)
set(PNG_EXECUTABLES OFF CACHE BOOL "" FORCE)
set(SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE)

add_subdirectory("./lib/libpng/")
include_directories(("./lib/libpng/"))
target_link_libraries(${TARGET_NAME} PRIVATE png_static)

target_compile_definitions(${TARGET_NAME} PRIVATE cimg_use_png)

add_library(miniz STATIC ./lib/miniz.c)
add_library(sqlite STATIC ./lib/sqlite3.c)

target_link_libraries(${TARGET_NAME} PRIVATE miniz sqlite)