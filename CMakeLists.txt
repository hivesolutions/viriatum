cmake_minimum_required(VERSION 3.15.0)
project(viriatum)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib)

add_definitions(-DNO_PRAGMA_LIB)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

file(GLOB_RECURSE VIRIATUM_COMMON_SOURCES src/viriatum_commons/*.c)
add_library(viriatum_commons ${VIRIATUM_COMMON_SOURCES})

target_include_directories(viriatum_commons PUBLIC src/viriatum_commons)

file(GLOB_RECURSE VIRIATUM_SOURCES src/viriatum/*.c)
add_executable(viriatum ${VIRIATUM_SOURCES})

target_include_directories(viriatum PUBLIC src/viriatum)
target_link_libraries(viriatum viriatum_commons ${CMAKE_DL_LIBS})
