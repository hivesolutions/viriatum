# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

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

* Migrated from Conan 1 to Conan 2 across all CMake CI jobs (GCC, Windows, macOS)
* Updated `conanfile.txt` generators from `cmake` to `CMakeToolchain` and `CMakeDeps`
* Removed Conan 1 specific `conanbuildinfo.cmake` integration from `CMakeLists.txt`
* Bumped `cmake_minimum_required` from 3.0.0 to 3.5.0
* Enabled `CMAKE_BUILD_TYPE=Release` for Linux and macOS CMake CI builds

### Fixed

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
