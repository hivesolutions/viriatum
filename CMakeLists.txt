cmake_minimum_required(VERSION 3.0.0)
project(viriatum)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_definitions(-DNO_PRAGMA_LIB)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

aux_source_directory(src/viriatum_commons VIRIATUM_COMMON_SOURCES)
add_library(viriatum_commons ${VIRIATUM_COMMON_SOURCES})

aux_source_directory(src/viriatum VIRIATUM_SOURCES)
add_executable(viriatum ${VIRIATUM_SOURCES})

target_link_libraries(viriatum ${CONAN_LIBS} viriatum_commons)
