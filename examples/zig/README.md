# Viriatum Zig Embedding Example

Embeds viriatum as a static library in a Zig application using `@cImport`
to consume the C89 headers directly — no bindings generator needed.

## Prerequisites

- [Zig](https://ziglang.org/download/) (0.13+)

## Build & Run

```bash
cd examples/zig
zig build run
```

The server starts on the default port (8080) using the built-in file handler.
Press `Ctrl+C` to stop.

## How It Works

The build system compiles viriatum's C sources into two static libraries
(`viriatum_commons` and `viriatum`) and links them into the Zig executable.

The `viriatum.c` file is excluded from the library because it contains
`main()`. Instead, `src/viriatum_shim.c` provides the globals that
`viriatum.c` normally defines, and `src/main.zig` calls the service
lifecycle functions directly:

```
create_service → load_specifications → load_options_service
→ calculate_options_service → start_service (blocks) → delete_service
```
