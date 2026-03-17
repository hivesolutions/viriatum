# Viriatum Zig Embedding Example

Embeds viriatum as a static library in a Zig application using manual
`extern` declarations for the C functions needed — no bindings generator
or `@cImport` required.

## Prerequisites

- [Zig](https://ziglang.org/download/) (0.15+)

## Build & Run

```bash
cd examples/zig
zig build run
```

The server starts on the default port (9090) using the dispatch handler.
Press `Ctrl+C` to stop.

## Tests

Run the full viriatum unit test suite (30 tests):

```bash
zig build test
```

## Release Builds

```bash
zig build -Doptimize=ReleaseFast    # optimized for speed
zig build -Doptimize=ReleaseSmall   # optimized for binary size (~125KB)
zig build -Doptimize=ReleaseSafe    # optimized with safety checks
```

## How It Works

The build system compiles viriatum's C sources into two static libraries
(`viriatum_commons` and `viriatum`) and links them into the Zig executable.

The `viriatum.c` file is excluded from the library because it contains
`main()`. Instead, `src/viriatum_shim.c` provides the globals that
`viriatum.c` normally defines, and `src/main.zig` calls the service
lifecycle functions directly.
