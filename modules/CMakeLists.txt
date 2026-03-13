set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/modules)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/modules)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/modules)

# determines the platform-specific module prefix and suffix,
# on Windows modules use no prefix (viriatum_mod_*.dll) while
# on Unix they use the lib prefix (libviriatum_mod_*.so/dylib)
# matching the VIRIATUM_MODULE_PREFIX constant in definitions.h
if(WIN32)
    set(MODULE_PREFIX "")
    set(MODULE_SUFFIX ".dll")
elseif(APPLE)
    set(MODULE_PREFIX "lib")
    set(MODULE_SUFFIX ".dylib")
else()
    set(MODULE_PREFIX "lib")
    set(MODULE_SUFFIX ".so")
endif()

option(VIRIATUM_BUILD_MOD_DIAG "Build the diagnostics module" ON)
if(VIRIATUM_BUILD_MOD_DIAG)
    add_subdirectory(mod_diag)
endif()

option(VIRIATUM_BUILD_MOD_GIF "Build the GIF module" ON)
if(VIRIATUM_BUILD_MOD_GIF)
    add_subdirectory(mod_gif)
endif()

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
