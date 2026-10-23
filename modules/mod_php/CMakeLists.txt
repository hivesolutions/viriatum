# detects PHP embed SAPI using php-config, the module requires
# the embed library (libphp) and sapi/embed/php_embed.h header
find_program(PHP_CONFIG php-config)
if(PHP_CONFIG)
    execute_process(COMMAND ${PHP_CONFIG} --prefix OUTPUT_VARIABLE PHP_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(WIN32)
        if(EXISTS "${PHP_PREFIX}/include/php/sapi/embed/php_embed.h")
            set(PHP_FOUND TRUE)
            execute_process(COMMAND ${PHP_CONFIG} --includes OUTPUT_VARIABLE PHP_INCLUDES_RAW OUTPUT_STRIP_TRAILING_WHITESPACE)
            string(REGEX MATCHALL "-I[^ ]+" PHP_INCLUDE_FLAGS "${PHP_INCLUDES_RAW}")
            set(PHP_INCLUDE_DIRS "")
            foreach(FLAG ${PHP_INCLUDE_FLAGS})
                string(SUBSTRING ${FLAG} 2 -1 DIR)
                list(APPEND PHP_INCLUDE_DIRS ${DIR})
            endforeach()
            find_library(PHP_LIBRARY NAMES php php8 php7 PATHS "${PHP_PREFIX}/lib")
        endif()
    else()
        # searches for the PHP embed library using platform-specific
        # extensions, macOS uses dylib while Unix uses .so
        set(PHP_LIB_CANDIDATES
            "${PHP_PREFIX}/lib/libphp.dylib"
            "${PHP_PREFIX}/lib/libphp.so"
        )
        set(PHP_LIB_FOUND FALSE)
        foreach(PHP_LIB_PATH ${PHP_LIB_CANDIDATES})
            if(EXISTS "${PHP_LIB_PATH}")
                set(PHP_LIB_FOUND TRUE)
                set(PHP_LIBRARY "${PHP_LIB_PATH}")
                break()
            endif()
        endforeach()

        # searches for the embed header under the include directory that
        # php-config reports, distributions version it by the API of PHP
        # (eg: include/php/20230831) rather than keeping it under the prefix
        execute_process(COMMAND ${PHP_CONFIG} --include-dir OUTPUT_VARIABLE PHP_INCLUDE_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
        if(EXISTS "${PHP_INCLUDE_DIR}/sapi/embed/php_embed.h" AND PHP_LIB_FOUND)
            set(PHP_FOUND TRUE)
            execute_process(COMMAND ${PHP_CONFIG} --includes OUTPUT_VARIABLE PHP_INCLUDES_RAW OUTPUT_STRIP_TRAILING_WHITESPACE)
            string(REGEX MATCHALL "-I[^ ]+" PHP_INCLUDE_FLAGS "${PHP_INCLUDES_RAW}")
            set(PHP_INCLUDE_DIRS "")
            foreach(FLAG ${PHP_INCLUDE_FLAGS})
                string(SUBSTRING ${FLAG} 2 -1 DIR)
                list(APPEND PHP_INCLUDE_DIRS ${DIR})
            endforeach()
        endif()
    endif()
endif()

if(PHP_FOUND)
    file(GLOB MOD_PHP_SOURCES src/viriatum_mod_php/*.c src/viriatum_mod_php/module/*.c)
    add_library(viriatum_mod_php MODULE ${MOD_PHP_SOURCES})
    set_target_properties(viriatum_mod_php PROPERTIES PREFIX "${MODULE_PREFIX}" SUFFIX ${MODULE_SUFFIX})
    target_include_directories(viriatum_mod_php PRIVATE
        src/viriatum_mod_php
        ${CMAKE_SOURCE_DIR}/src/viriatum
        ${CMAKE_SOURCE_DIR}/src/viriatum_commons
        ${PHP_INCLUDE_DIRS}
    )
    target_link_libraries(viriatum_mod_php viriatum_commons ${PHP_LIBRARY})
    message(STATUS "Viriatum: mod_php enabled")

    # test executable: compiles the module sources together with
    # the test files into a standalone binary that can be invoked
    # to run the mod_php test suite
    file(GLOB MOD_PHP_TEST_SOURCES src/viriatum_mod_php/test/*.c)
    add_executable(viriatum_mod_php_test
        ${MOD_PHP_SOURCES}
        ${MOD_PHP_TEST_SOURCES}
        ${CMAKE_SOURCE_DIR}/src/viriatum/http/http_parser.c
        ${CMAKE_SOURCE_DIR}/src/viriatum/http/http_request.c
    )
    set_target_properties(viriatum_mod_php_test PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin
    )
    target_include_directories(viriatum_mod_php_test PRIVATE
        src/viriatum_mod_php
        ${CMAKE_SOURCE_DIR}/src/viriatum
        ${CMAKE_SOURCE_DIR}/src/viriatum_commons
        ${PHP_INCLUDE_DIRS}
    )
    target_link_libraries(viriatum_mod_php_test viriatum_static viriatum_commons ${PHP_LIBRARY})
    target_compile_definitions(viriatum_mod_php_test PRIVATE
        VIRIATUM_TEST_CONTENTS_PATH="${CMAKE_SOURCE_DIR}/modules/mod_php/resources/test"
    )
else()
    message(STATUS "Viriatum: mod_php disabled (PHP embed SAPI not found)")
endif()
