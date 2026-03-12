set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/modules)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/modules)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/modules)

# determines the platform-specific module suffix, on Windows
# modules are built as DLLs while on macOS they use dylib and
# on other Unix systems they use the standard .so extension
if(WIN32)
    set(MODULE_SUFFIX ".dll")
elseif(APPLE)
    set(MODULE_SUFFIX ".dylib")
else()
    set(MODULE_SUFFIX ".so")
endif()

# standalone modules that have no external dependencies are
# always built when module support is enabled
option(VIRIATUM_BUILD_MOD_DIAG "Build the diagnostics module" ON)
if(VIRIATUM_BUILD_MOD_DIAG)
    add_subdirectory(mod_diag)
endif()

option(VIRIATUM_BUILD_MOD_GIF "Build the GIF module" ON)
if(VIRIATUM_BUILD_MOD_GIF)
    add_subdirectory(mod_gif)
endif()

# modules that depend on external libraries are only built
# when the required dependencies are found on the system
option(VIRIATUM_BUILD_MOD_PHP "Build the PHP module" ON)
if(VIRIATUM_BUILD_MOD_PHP)
    add_subdirectory(mod_php)
endif()

option(VIRIATUM_BUILD_MOD_LUA "Build the Lua module" ON)
if(VIRIATUM_BUILD_MOD_LUA)
    add_subdirectory(mod_lua)
endif()

option(VIRIATUM_BUILD_MOD_WSGI "Build the WSGI module" ON)
if(VIRIATUM_BUILD_MOD_WSGI)
    add_subdirectory(mod_wsgi)
endif()
