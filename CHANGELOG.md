# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

* Path traversal validation (`is_path_safe`) in `viriatum_commons/util/string_util.h`
* Unit test `test_is_path_safe` covering traversal detection and false positive avoidance

### Changed

*

### Fixed

* Reject URLs containing `..` path traversal sequences in file handler to prevent access outside the web root
