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
else()
    message(STATUS "Viriatum: mod_lua disabled (Lua not found)")
endif()
