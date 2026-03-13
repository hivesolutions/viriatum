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

option(VIRIATUM_DEBUG "Enable debug logging and diagnostics" OFF)
if(VIRIATUM_DEBUG OR CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DHAVE_DEBUG)
    message(STATUS "Viriatum: debug mode enabled")
endif()

set(VIRIATUM_WWW_ROOT "" CACHE STRING "Path to the www root directory (sets WITH_WWW_ROOT)")
if(VIRIATUM_WWW_ROOT)
    add_definitions(-DWITH_WWW_ROOT="${VIRIATUM_WWW_ROOT}")
    message(STATUS "Viriatum: www root set to '${VIRIATUM_WWW_ROOT}'")
endif()

include(CheckStructHasMember)
include(CheckSymbolExists)

# checks for IPv6 support mirroring the autoconf ipv6.m4 logic:
# requires sockaddr_in6, getaddrinfo and getnameinfo, on Windows
# the _config_win32.h header already defines HAVE_IP6 so we only
# need to run the detection on Unix systems
if(NOT WIN32)
    check_struct_has_member("struct sockaddr_in6" sin6_addr "netinet/in.h" HAVE_SOCKADDR_IN6)
    check_symbol_exists(getaddrinfo "sys/types.h;sys/socket.h;netdb.h" HAVE_GETADDRINFO)
    check_symbol_exists(getnameinfo "sys/types.h;sys/socket.h;netdb.h" HAVE_GETNAMEINFO)
    if(HAVE_SOCKADDR_IN6 AND HAVE_GETADDRINFO AND HAVE_GETNAMEINFO)
        add_definitions(-DHAVE_IP6)
    endif()
endif()

# passes the effective C flags as a preprocessor define so the
# compilation flags banner can display them at runtime
if(CMAKE_BUILD_TYPE)
    string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)
    string(STRIP "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE_UPPER}}" VIRIATUM_CFLAGS)
else()
    string(STRIP "${CMAKE_C_FLAGS}" VIRIATUM_CFLAGS)
endif()
add_definitions(-DCFLAGS="${VIRIATUM_CFLAGS}")

file(GLOB_RECURSE VIRIATUM_COMMON_SOURCES src/viriatum_commons/*.c)
add_library(viriatum_commons ${VIRIATUM_COMMON_SOURCES})
set_target_properties(viriatum_commons PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(viriatum_commons PUBLIC src/viriatum_commons)

# links winsock and process status API on Windows, these
# are normally auto-linked via pragma comments in the
# headers but NO_PRAGMA_LIB disables that mechanism
if(WIN32)
    target_link_libraries(viriatum_commons ws2_32 psapi)
endif()

file(GLOB_RECURSE VIRIATUM_SOURCES src/viriatum/*.c)
add_executable(viriatum ${VIRIATUM_SOURCES})

target_include_directories(viriatum PUBLIC src/viriatum)
target_compile_definitions(viriatum PRIVATE WITH_MODULE_ROOT="${CMAKE_BINARY_DIR}/modules")
target_link_libraries(viriatum viriatum_commons)

# links the dynamic loader on Unix systems
if(NOT WIN32)
    target_link_libraries(viriatum ${CMAKE_DL_LIBS})
endif()

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PCRE libpcre QUIET)
endif()
if(PCRE_FOUND)
    target_compile_definitions(viriatum PRIVATE VIRIATUM_PCRE HAVE_PCRE_H)
    target_include_directories(viriatum PRIVATE ${PCRE_INCLUDE_DIRS})
    target_link_libraries(viriatum ${PCRE_LINK_LIBRARIES})
endif()

# modules are built as shared libraries loaded at runtime,
# the VIRIATUM_BUILD_MODULES option controls whether any
# modules are built and individual options allow fine-grained
# control over each module
option(VIRIATUM_BUILD_MODULES "Build viriatum modules" ON)
if(VIRIATUM_BUILD_MODULES)
    add_subdirectory(modules)
endif()
