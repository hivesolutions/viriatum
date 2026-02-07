# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

* Path traversal validation (`is_path_safe`) in `viriatum_commons/util/string_util.h`
* Unit test `test_is_path_safe` covering traversal detection and false positive avoidance
* Dedicated `handler_file_test.c` with tests for file handler context, URL parsing, header fields, and header values

### Changed

* Migrated from Conan 1 to Conan 2 across all CMake CI jobs (GCC, Windows, macOS)
* Updated `conanfile.txt` generators from `cmake` to `CMakeToolchain` and `CMakeDeps`
* Removed Conan 1 specific `conanbuildinfo.cmake` integration from `CMakeLists.txt`
* Bumped `cmake_minimum_required` from 3.0.0 to 3.5.0

### Fixed

* Reject URLs containing `..` path traversal sequences in file handler to prevent access outside the web root
* Fixed leaked `#pragma pack(1)` in `stream_torrent.h` causing struct alignment mismatches across compilation units
