cmake_minimum_required(VERSION 3.0.0)
project(viriatum)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_definitions(-DNO_PRAGMA_LIB)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

file(GLOB_RECURSE VIRIATUM_COMMON_SOURCES src/viriatum_commons/*.c)
add_library(viriatum_commons ${VIRIATUM_COMMON_SOURCES})

target_include_directories(viriatum_commons PUBLIC src/viriatum_commons)

file(GLOB_RECURSE VIRIATUM_SOURCES src/viriatum/*.c)
add_executable(viriatum ${VIRIATUM_SOURCES})

target_include_directories(viriatum PUBLIC src/viriatum)
target_link_libraries(viriatum viriatum_commons ${CMAKE_DL_LIBS} ${CONAN_LIBS})
