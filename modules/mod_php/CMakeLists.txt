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
        if(EXISTS "${PHP_PREFIX}/include/php/sapi/embed/php_embed.h" AND PHP_LIB_FOUND)
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
else()
    message(STATUS "Viriatum: mod_php disabled (PHP embed SAPI not found)")
endif()
