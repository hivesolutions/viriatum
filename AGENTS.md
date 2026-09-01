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

## Running

The binary runs a useful server from its arguments alone, no file on disk is required for any of it:

```bash
./bin/viriatum --file=. --bind=:8080
./bin/viriatum --file=./dist --spa --cors --dev
./bin/viriatum --file=. --port=0
```

The parser is flag based, a value travels after an equals sign, there is no positional argument and no sub command, so `viriatum .` is an error and a flag given a value with a space is not read as one. The short forms are `-p` for the port and `-h` for the host, help is reachable as `--help` alone.

`--asgi` and `--wsgi` are the exception to all of that: they normalise the target they are given, onto a module, an attribute and the directory that has to be reachable for the import, and then report that this binary is not the thing that serves it. The two handlers that do live in the extension under `src/viriatum_python` and are handed an application the interpreter has already imported, while the binary embeds no interpreter of its own and so has nothing to hand them; `viriatum.serve` is what serves an application today. The guard `VIRIATUM_PYTHON` in `src/viriatum/system/service.c` marks where that would be answered otherwise and is defined by no build.

The options are layered, the defaults first, then the configuration file, then the command line, so the flags complement a file rather than replace one. Which file is read is decided by `--config`, which names one and fails outright when it is not there, and by `--no-config`, which skips the discovery altogether; with neither of them the first of the three paths that `--help` lists is the one that is read. A flag that selects what is served, `--file`, `--asgi` or `--wsgi`, conflicts with a `--handler` that asks for another one and the run is refused naming both, so neither of them ever wins quietly.

`--print-config` writes the configuration the three layers merged into and `--check` validates everything a run is made of, both of them without ever binding a socket, which is what a change to any of the layering should be verified through. `--list-handlers` writes the handlers the build carries, the ones of the modules included.

The harness of the benchmark is the first real user of all of this and drives the subject entirely from these flags, the proxy workloads excepted, an upstream being a location and a location being written down.

## Adding a Source File

CMake gathers the sources of the tree through a pattern and so it picks a new file up on its own, every one of the other build systems names them one by one and has to be told. When a source or a header is added, removed or renamed, the following have to be kept in step with it:

- `src/viriatum/Makefile.am` and `src/viriatum_commons/Makefile.am`, each of them carrying two lists of sources, the ones of the binary or of the static library and the ones of the shared library
- `src/Makefile.am`, which carries the headers of both trees that the packaging installs
- `win32/vs2015/viriatum.vcxproj` and `win32/vs2015/viriatum_commons.vcxproj`, along with the `.filters` beside each of them that says which group a file belongs to
- `win32/vs2015/viriatum.vcproj`, `win32/vs2015/viriatum_commons.vcproj` and the pair of the same names under `win32/vs2008ex`, which are the older format of those very projects
- `win32/vs2008ex/viriatum_mod_*.vcproj` and `win32/vs2015/viriatum_mod_*.vcproj`, for a source of a module
- `darwin/xcode4/viriatum/viriatum.xcodeproj/project.pbxproj` and the one of the commons beside it, which name a file in four places, the reference of it, the group it hangs from and the build of it for each of the two targets
- `examples/zig/build.zig`, which carries a list of its own

A suite carries two more, `src/viriatum/test/simple_test.c` has to include the header of it and to name every one of its tests in the registry that closes that file.

The packaging for Python builds through CMake and so needs nothing of its own. The fuzz targets are the single deliberate exception to all of the above, they carry an entry point of their own and are named by no project.

## Formatting

Format C code before committing using clang-format with the project's `.clang-format` configuration:

```bash
clang-format -i <file>
```

The whole tree is checked at once through a script of its own, which reports every source the formatter would rewrite and fails when there is one, and which formats them in place when asked to:

```bash
./scripts/format.sh
./scripts/format.sh --fix
```

A job of the integration runs the first of those on every push, so a source that is left unformatted fails the build rather than passing quietly. The formatter is pinned there to a single version, the output of it differs between versions and a drifting one would make the job flip on its own.

## Testing

Run the core test suite and module tests before committing:

```bash
./bin/viriatum --test
./bin/viriatum_mod_lua_test
```

The mod_lua test binary is only built when LuaJIT is available on the system.

Tests are described as data, one row per test in a registry table, so that they may be listed, filtered and reported without any of them having to be named in an execution sequence. The table of the core suite lives in `src/viriatum/test/simple_test.c` and a test is added to it by writing the function, declaring it in the matching `*_test.h` and adding a row:

```c
V_TEST_T(test_my_feature, "handler"),
```

The available row constructors are `V_TEST` (no tags), `V_TEST_T` (tagged), `V_TEST_M` (tagged plus the `TEST_FLAG_SKIP` or `TEST_FLAG_XFAIL` marker), `V_TEST_C` (with a setup and a teardown fixture), `V_TEST_P` (one execution per parameter case) and `V_TEST_S` (a speed test with a count of iterations). A fixture is preferred over building the structures inside the test body, its teardown also runs when an assertion fails midway.

The runner is driven through the following options:

```bash
./bin/viriatum --test --test-list
./bin/viriatum --test --test-filter=websocket
./bin/viriatum --test --test-tags=structures,path
./bin/viriatum --test --test-format=junit --test-output=test-results.xml
```

The formats of the report are `text`, `tap`, `junit` and `markdown`. A format given without an output path turns the echoing of the progress off, but the tests themselves still write to the standard output, so a report that is meant to be parsed should always be directed to a file through `--test-output`. The assertions that report both of the values (`V_ASSERT_EQ_I`, `V_ASSERT_EQ_U`, `V_ASSERT_EQ_S`, `V_ASSERT_EQ_P`, `V_ASSERT_NULL`, `V_ASSERT_NOT_NULL` and `V_ASSERT_MEM`) are preferred over the plain `V_ASSERT` wherever a comparison is involved.

## Coverage

Measure the coverage of the C tree and of the python package with:

```bash
./scripts/coverage.sh
```

The script builds every target with instrumentation, runs the core, the module and the python suites and writes the reports under `coverage/`. Both clang, through llvm-cov, and gcc, through gcovr, are supported and produce the same set of reports. The threshold of the C tree is set through `THRESHOLD` and the one of the python surface through `PYTHON_THRESHOLD`, raise them as the coverage improves rather than lowering them to make a run pass.

## Memory

The memory of the tree is measured by driving the suite under the address sanitizer:

```bash
./scripts/sanitize.sh
```

An error of the memory fails the run outright, whatever its shape, and the allocations that are left behind are compared against `scripts/sanitize.baseline`, which a run above fails. The number is lowered as the leaks are closed and never raised to make a run pass, the very same rule the conformance and the coverage are held to.

The leak part of the sanitizer does not run on macOS with the compiler that ships with the system, so the job of the integration is what measures the leaks by default. A compiler that does carry the detector may be driven through it, which is how a leak is chased without waiting for a run of the integration:

```bash
CC=/opt/homebrew/opt/llvm/bin/clang LEAKS=1 ./scripts/sanitize.sh
```

The runner counts the allocations each test leaves outstanding and lists the ones that left the most at the end of a run, which is what points at the test carrying a leak. An allocation that is still outstanding is not necessarily a leak, a value built by one test may well be released by a later one, so the listing narrows the search and the sanitizer settles it. The server counts its own allocations in a debug build too and reports the outstanding ones when the process ends.

## Performance

The serving is measured against the reference servers by a harness that starts them all on the same machine, drives them through the same workloads and reports them side by side:

```bash
./scripts/benchmark.sh
ONLY=static-small-alive ./scripts/benchmark.sh
```

Absolute numbers from any one machine are not comparable to another, so the figure that is tracked is the **ratio of the server against a reference measured in the same run**. Every run is compared against `scripts/benchmark/baseline.json`, which is only ever refreshed through an explicit input of the workflow so that it cannot quietly ratchet down. The machine and the shape of the load are part of what makes two runs comparable, so a change to `CONNECTIONS`, `THREADS` or `WORKERS` leaves the stored baseline behind and the change column empty until it is refreshed under the new shape, which the report says outright. The run reports and never gates, a hosted runner being far too noisy to fail a build on a performance figure.

A claim about performance needs a run of the harness behind it. An optimisation lands with a before and after attached and is reverted when the gain does not hold, and a change is measured by driving the two binaries interleaved rather than in blocks, so that a drift of the machine lands on both of them equally. Establish where the time goes with a profile before changing anything: the widest gap is rarely where it is assumed to be, and an entry in `doc/todo.md` is a hypothesis rather than a finding.

The service waits on its connections through `epoll` where it exists, `kqueue` where it does not, and `select` only when neither is around. The one in use is named in the banner of the startup, in the flags that travel on every response, and on the status page. `select` walks every descriptor on every pass and stops working entirely past `FD_SETSIZE`, which is 1024, so a build that falls back on it is a build that will not hold more than a thousand connections; drive `CONNECTIONS=1200` and `CONNECTIONS=2000` against it before assuming otherwise, one run of the harness for each of them.

The files that the file handler serves are kept open in a cache of its own, one per worker process, so that serving a file again costs neither the opening of it nor the describing of it. The size of a held file is taken from its descriptor on every request, so a file written over in place is always served at the length it now has; a file **replaced** at the same path is picked up once the entry is looked at again, which is `CACHE_VALID_HANDLER_FILE` seconds at the latest. A path falls on exactly one entry, decided by the hash of it, and takes that entry over from whatever was there before.

The methodology, the configuration each server is given and the reasoning behind every one of those choices are written down in `scripts/benchmark/README.md`. Anything that affects the comparison, such as the logging of a request or the pooling of an upstream connection, belongs there the moment it is changed.

## HTTP/2

Both versions of the protocol are served on the same port and are told apart by the bytes that open a connection, the preface handing it to a session of HTTP/2 and anything else to the parser of HTTP/1.1. Over the transport the version is negotiated through ALPN instead, honouring the order the client announces.

A handler never knows which of the two is serving it. The message it is driven for is `struct http_request_t`, populated either by the parser or by the decoding of a header block, and the response is written through the operations that `struct http_connection_t` carries, `write_status`, `write_field`, `write_line`, `write_end`, `write_chunk` and `write_flush`, which frame it according to the version in use. A handler that writes bytes straight to the connection is only ever correct for HTTP/1.1.

The layers are `src/viriatum/http/hpack.c` for the compression of the header fields, `src/viriatum/http/http2.c` for the frames and `src/viriatum/stream/stream_http2.c` for the session, the streams and the flow control, the last one mirroring the shape of `stream_http.c`.

The support is built unless the build is told otherwise, through `--disable-http2` under Autoconf or `-D VIRIATUM_HTTP2=OFF` under CMake, and the flag it defines shows up in the banner of the startup. The code of it is guarded by `VIRIATUM_HTTP2`, so a call into any of the three layers above belongs inside that guard.

The conformance of the implementation is measured against `h2spec` and the interoperability against the clients a deployment meets:

```bash
./scripts/conformance.sh
./scripts/interop.sh
```

The first compares the cases that pass against `scripts/conformance.baseline` and fails a run below it, so the number is raised as the implementation improves rather than lowered to make a run pass. The decoders of the frames and of the header blocks are the two places that take apart what a peer controls and are driven by an engine of fuzzing:

```bash
./scripts/fuzz.sh
```

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

- [ ] Every build system names a new source file, see [Adding a Source File](#adding-a-source-file)
- [ ] Code is formatted with `clang-format`, which `./scripts/format.sh` verifies over the whole tree
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
  - `pyproject.toml` — update the `version` field, it is the one published to PyPI
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
