# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

* WSGI module introspection methods: `connections()`, `connections_l()`, `connection_info()`, and `uptime()` for runtime server state inspection
* WSGI module constants (`NAME`, `VERSION`, `PLATFORM`, `FLAGS`, `MODULES`, `DESCRIPTION`, `COMPILER`, `COMPILER_VERSION`, `COMPILATION_DATE`, `COMPILATION_TIME`, `COMPILATION_FLAGS`) exposed via `viriatum_wsgi` module
* Enhanced handler.wsgi and handler.lua demo pages with server info (engine, version, platform, compiler, uptime, connections)
* Path traversal validation (`is_path_safe`) in `viriatum_commons/util/string_util.h`
* Unit test `test_is_path_safe` covering traversal detection and false positive avoidance
* Dedicated `handler_file_test.c` with tests for file handler context, URL parsing, header fields, and header values
* Warning-level logging for HTTP errors in `write_http_error_a` and dispatch handler 500 responses
* Request path included in dispatch error log messages for easier debugging
* Module load logging at startup using `V_PRINT_F` showing module name, version and path
* Multi-stage Docker builds for both `Dockerfile` (11 MB) and `Dockerfile.php` (22 MB)
* `Dockerfile.all` with all 5 modules (diag, gif, lua, php, wsgi) using 4-stage multi-stage build (112 MB)
* SSL support enabled in `Dockerfile` and `Dockerfile.php` builds
* Documentation for `upgrade` and `context` fields in `http_parser_t`
* Compilation flags shown in startup banner (e.g. `[nts ipv6 pcre]`)
* Regression test for dispatch handler context lifecycle on keep-alive connections
* Warning-level logging in mod_lua, mod_php, and mod_wsgi for 500 errors with request context
* Line-buffered stdout/stderr via `setvbuf` to ensure log output is visible when piped (e.g. Docker)

### Changed

* Upgraded mod_wsgi from Python 2.7 to Python 3 (PyGILState API, modern embed workflow)
* Upgraded mod_php from PHP 5.6 to PHP 8.x (libphp.so, updated header paths)
* `Dockerfile.all` rewritten: uses Alpine system packages (python3-dev, php84-embed, lua5.1-dev) instead of building Python 2.7 and PHP 5.6 from source
* `Dockerfile.php` updated from PHP 5.6 source build to php84-embed Alpine package
* Static `file_path` buffers in mod_lua and mod_wsgi handler structs (eliminates MALLOC/FREE for path strings)
* ASCII art banner and runtime diagnostics in handler.lua, handler.wsgi, and phpinfo page
* Module documentation in README updated for Python 3, PHP 8, and modern package managers
* Autoconf m4 macros modernized: mod_php detects `libphp` (PHP 8+), mod_wsgi auto-detects Python 3 via `python3-config`, mod_lua tries multiple library names across distros
* Header detection in m4 switched from `AC_CHECK_HEADERS` to `AC_COMPILE_IFELSE` for PHP, Python, and Lua (fixes false negatives with complex headers)
* Added `www_root` configuration option to override the compile-time web root for file serving
* Migrated from Conan 1 to Conan 2 across all CMake CI jobs (GCC, Windows, macOS)
* Updated `conanfile.txt` generators from `cmake` to `CMakeToolchain` and `CMakeDeps`
* Removed Conan 1 specific `conanbuildinfo.cmake` integration from `CMakeLists.txt`
* Bumped `cmake_minimum_required` from 3.0.0 to 3.5.0
* Enabled `CMAKE_BUILD_TYPE=Release` for Linux and macOS CMake CI builds

### Fixed

* Replaced deprecated `Py_SetProgramName`/`Py_Initialize` with `PyConfig` API in mod_wsgi (fixes Python 3.11+ deprecation warning)
* Fixed `DESCRIPTION` module constant in mod_wsgi pointing to `compiler` field instead of `description`
* Fixed Python object reference leaks in `wsgi_connections_l` and `wsgi_connection_info` (missing `Py_DECREF` after `PyDict_SetItemString`)
* Reject URLs containing `..` path traversal sequences in file handler to prevent access outside the web root
* Fixed leaked `#pragma pack(1)` in `stream_torrent.h` causing struct alignment mismatches across compilation units
* Fixed potential memory leak of dispatch handler path context on early connection close
* Fixed IPv6 autodetection failing with GCC 15 (C23) due to `void main` and missing `arpa/inet.h` in m4 network checks
* Fixed double-free of `http_parser->context` in dispatch handler unset causing server hang on keep-alive connections
* File and proxy handler unsets now NULL `http_parser->context` after freeing to prevent dangling pointers
* PHP and WSGI module handler unsets now NULL `http_parser->context` after freeing
* Fixed `REQUEST_URI` length using path size instead of full URL size in mod_php handler
* Connection info page now shows "not found" message instead of empty fields for closed connections
* Fixed `setvbuf` crash on Windows/MSVC by using `_IONBF` (unbuffered) instead of `_IOLBF` which rejects size 0
* Fixed error and directory listing templates not respecting `www_root` config (was using compile-time resources path)
* Fixed multiple Python C API reference leaks in mod_wsgi on error paths (handler, extension, entry)
* Fixed missing GIL release on type-error path in mod_wsgi response data callback
* Fixed `zend_string` memory leak in mod_php INI entry setup (missing `zend_string_release`)
