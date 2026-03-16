# AGENTS.md

## Building

The primary build system is CMake. To build Viriatum with all available modules:

```bash
cmake . -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
make
```

Dependencies are managed via Conan 2:

```bash
pip3 install --upgrade "conan>=2,<3" urllib3
conan profile detect --force
conan install . --build=missing
```

## Formatting

Format C code before committing using clang-format with the project's `.clang-format` configuration:

```bash
clang-format -i <file>
```

## Testing

Run the core test suite and module tests before committing:

```bash
./bin/viriatum --test
./bin/viriatum_mod_lua_test
```

The mod_lua test binary is only built when LuaJIT is available on the system.

## Style Guide

- C source files use 4-space indentation, no tabs.
- Braces follow K&R style: opening brace on the same line as the statement.
- No space before parentheses in control structures: `if(condition)`, `for(...)`, `while(...)`.
- Pointer declarations use right alignment: `struct service_t *service`.
- Comments use block style only (`/* ... */`), never C++ style (`//`).
- The commenting style of the project is unique and verbose, try to keep commenting style consistent.
- Naming uses `snake_case` for functions and variables, `UPPER_CASE` for macros and constants, and structs use the `_t` suffix (e.g., `struct service_t`).
- Line length should not exceed 80 characters.
- Single-line if blocks are allowed: `if(condition) { statement; }`.
- C source files use CRLF as the line ending.
- All source files begin with the Hive Solutions license header block.
- The implementation should be done in a way that is compatible with the existing codebase.
- Always update `CHANGELOG.md` according to semantic versioning, mentioning your changes in the unreleased section.

## Commit Messages

This project follows [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/) with the following structure:

```text
<type>: <description>

<body>
```

### Commit Types

| Type       | Description                                             |
| ---------- | ------------------------------------------------------- |
| `feat`     | A new feature or functionality                          |
| `fix`      | A bug fix                                               |
| `docs`     | Documentation only changes                              |
| `refactor` | Code change that neither fixes a bug nor adds a feature |
| `chore`    | Maintenance tasks, dependency updates, build changes    |
| `test`     | Adding or updating tests                                |
| `version`  | Version bump commits (reserved for releases)            |

### Guidelines

- Use lowercase for the type prefix.
- Use imperative mood in the description (e.g., "add feature" not "added feature").
- Keep the first line under 50 characters.
- Reference issue/PR numbers when applicable using `(#123)` at the end.
- For version releases, use the format `version: X.Y.Z`.
- Add an extra newline between subject and body.
- Make the body a series of bullet points about the commit.
- Be descriptive always making use of the body of the message.

### Examples

```text
feat: add WSAPI handler to mod_lua (#35)
fix: resolve double-free in dispatch handler
docs: update module documentation for Python 3
refactor: extract validation logic into string_util
chore: update Conan dependencies to latest versions
test: add integration tests for mod_lua WSAPI handler
version: 0.4.0
```

## Pre-Commit Checklist

Before committing, ensure that the following items check:

- [ ] Code is formatted with `clang-format`
- [ ] Tests pass: `./bin/viriatum --test`
- [ ] Module tests pass (if applicable): `./bin/viriatum_mod_lua_test`
- [ ] CHANGELOG.md is updated in [Unreleased] section
- [ ] No debugging print statements or commented-out code
- [ ] CRLF line endings are preserved
- [ ] License header is present in all new source files

## New Release

To create a new release follow the following steps:

- Make sure that both the tests pass and the code formatting is valid.
- Increment (look at `CHANGELOG.md` for semver changes) the version in the following files:
  - `src/viriatum/global/definitions.h` — update `VIRIATUM_MAJOR`, `VIRIATUM_MINOR`, `VIRIATUM_MICRO`, `VIRIATUM_STAGE_NAME`, and `VIRIATUM_STAGE_INDEX`
  - `scripts/build/build.json` — update the `"version"` field
  - `m4/settings.m4` — update `viriatum_major_version`, `viriatum_minor_version`, `viriatum_micro_version`
  - `src/viriatum_commons/stdafx.c` — update the `version[]` string
  - Each module's `m4/settings.m4` and `stdafx.c` version string (under `modules/mod_diag/`, `modules/mod_gif/`, `modules/mod_lua/`, `modules/mod_php/`, `modules/mod_wsgi/`)
- Move all the `CHANGELOG.md` Unreleased items that have at least one non empty item into a new section with the new version number and date, and then create new empty sub-sections (Added, Changed and Fixed) for the Unreleased section with a single empty item.
- Create a commit with the following message `version: $VERSION_NUMBER`.
- Push the commit.
- Create a new tag with the value of the new version number `$VERSION_NUMBER`.
- Create a new release on the GitHub repo using the Markdown from the corresponding version entry in `CHANGELOG.md` as the description of the release and the version number as the title. Do not include the title of the release (version and date) in the description.

## License

Hive Viriatum is licensed under the [Apache License, Version 2.0](http://www.apache.org/licenses/).
