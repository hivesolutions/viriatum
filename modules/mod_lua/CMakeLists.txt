find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(LUA luajit QUIET)
    if(NOT LUA_FOUND)
        pkg_check_modules(LUA lua5.1 QUIET)
    endif()
    if(NOT LUA_FOUND)
        pkg_check_modules(LUA lua QUIET)
    endif()
endif()

# fallback: try to locate LuaJIT or Lua headers/libs directly when
# pkg-config is unavailable (common on Windows)
if(NOT LUA_FOUND)
    find_path(LUA_INC_DIR lua.h
        PATH_SUFFIXES luajit-2.1 lua5.1 lua
        HINTS ENV LUA_DIR $ENV{USERPROFILE}/scoop/apps/luajit/current/include
    )
    find_library(LUA_LIB
        NAMES luajit-5.1 lua51 lua5.1 lua
        HINTS ENV LUA_DIR $ENV{USERPROFILE}/scoop/apps/luajit/current/lib
              $ENV{USERPROFILE}/scoop/apps/luajit/current/bin
    )
    if(LUA_INC_DIR AND LUA_LIB)
        set(LUA_FOUND TRUE)
        set(LUA_INCLUDE_DIRS ${LUA_INC_DIR})
        set(LUA_LINK_LIBRARIES ${LUA_LIB})
    endif()
endif()

if(LUA_FOUND)
    file(GLOB MOD_LUA_SOURCES src/viriatum_mod_lua/*.c src/viriatum_mod_lua/module/*.c)
    add_library(viriatum_mod_lua MODULE ${MOD_LUA_SOURCES})
    set_target_properties(viriatum_mod_lua PROPERTIES PREFIX "${MODULE_PREFIX}" SUFFIX ${MODULE_SUFFIX})
    target_include_directories(viriatum_mod_lua PRIVATE
        src/viriatum_mod_lua
        ${CMAKE_SOURCE_DIR}/src/viriatum
        ${CMAKE_SOURCE_DIR}/src/viriatum_commons
        ${LUA_INCLUDE_DIRS}
    )
    target_link_libraries(viriatum_mod_lua viriatum_commons ${LUA_LINK_LIBRARIES})
    target_compile_options(viriatum_mod_lua PRIVATE ${LUA_CFLAGS_OTHER})

    # the module includes headers as <lua5.1/lua.h> so we create a
    # compatibility symlink when the system uses a different path
    foreach(LUA_INC_DIR ${LUA_INCLUDE_DIRS})
        get_filename_component(LUA_INC_PARENT ${LUA_INC_DIR} DIRECTORY)
        get_filename_component(LUA_INC_NAME ${LUA_INC_DIR} NAME)
        if(NOT LUA_INC_NAME STREQUAL "lua5.1" AND NOT EXISTS "${LUA_INC_PARENT}/lua5.1")
            file(CREATE_LINK ${LUA_INC_DIR} ${CMAKE_BINARY_DIR}/lua5.1 SYMBOLIC)
            target_include_directories(viriatum_mod_lua PRIVATE ${CMAKE_BINARY_DIR})
        endif()
    endforeach()
    message(STATUS "Viriatum: mod_lua enabled")

    # test executable: compiles the module sources together with
    # the test files into a standalone binary that can be invoked
    # to run the mod_lua test suite
    file(GLOB MOD_LUA_TEST_SOURCES src/viriatum_mod_lua/test/*.c)
    add_executable(viriatum_mod_lua_test
        ${MOD_LUA_SOURCES}
        ${MOD_LUA_TEST_SOURCES}
        ${CMAKE_SOURCE_DIR}/src/viriatum/http/http_parser.c
    )
    set_target_properties(viriatum_mod_lua_test PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin
    )
    target_include_directories(viriatum_mod_lua_test PRIVATE
        src/viriatum_mod_lua
        ${CMAKE_SOURCE_DIR}/src/viriatum
        ${CMAKE_SOURCE_DIR}/src/viriatum_commons
        ${LUA_INCLUDE_DIRS}
    )
    target_link_libraries(viriatum_mod_lua_test viriatum_commons ${LUA_LINK_LIBRARIES})
    target_compile_options(viriatum_mod_lua_test PRIVATE ${LUA_CFLAGS_OTHER})
    target_compile_definitions(viriatum_mod_lua_test PRIVATE
        VIRIATUM_TEST_SCRIPT_PATH="${CMAKE_SOURCE_DIR}/src/viriatum/resources/html/base/test_handler.lua"
    )

    # reuses the lua5.1 compatibility symlink for the test target
    foreach(LUA_INC_DIR ${LUA_INCLUDE_DIRS})
        get_filename_component(LUA_INC_NAME ${LUA_INC_DIR} NAME)
        if(NOT LUA_INC_NAME STREQUAL "lua5.1")
            target_include_directories(viriatum_mod_lua_test PRIVATE ${CMAKE_BINARY_DIR})
        endif()
    endforeach()
else()
    message(STATUS "Viriatum: mod_lua disabled (Lua not found)")
endif()
