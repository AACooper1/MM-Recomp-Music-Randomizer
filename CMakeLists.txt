cmake_minimum_required(VERSION 3.20)

project(recomp_mod_native_lib)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TARGET_NAME $ENV{LIB_NAME})

include_directories("./offline_build")
include_directories("./include/extlib")
include_directories("./include/mod")
include_directories("./src/extlib")
include_directories("./src/extlib/database")


add_library(${TARGET_NAME} SHARED)

add_subdirectory("./src/extlib")
if (CMAKE_BUILD_TYPE MATCHES ".*Debug")
    include_directories("./tests/src/extlib")
    include_directories("./tests/lib")

    add_subdirectory("./tests/src/extlib")
    add_subdirectory("./tests/lib")
endif()

set_target_properties(${TARGET_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "./arc/"
    LIBRARY_OUTPUT_DIRECTORY "./lib/"
    RUNTIME_OUTPUT_DIRECTORY "./lib/"
)
