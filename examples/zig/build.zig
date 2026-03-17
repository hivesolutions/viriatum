const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // root of the viriatum source tree relative to this build file
    const viriatum_root = b.path("../../");

    const c_flags: []const []const u8 = if (target.result.os.tag == .windows)
        &.{ "-DNO_PRAGMA_LIB", "-DCFLAGS=\"\"", "-D_CRT_SECURE_NO_WARNINGS", "-DEINTR=4", "-fcommon" }
    else
        &.{ "-DNO_PRAGMA_LIB", "-DCFLAGS=\"\"" };

    // --- viriatum_commons (static library) ---

    const commons_mod = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    commons_mod.addIncludePath(viriatum_root.path(b, "src/viriatum_commons"));
    commons_mod.addCSourceFiles(.{
        .root = viriatum_root.path(b, "src/viriatum_commons"),
        .files = &commons_sources,
        .flags = c_flags,
    });
    if (target.result.os.tag == .windows) {
        commons_mod.linkSystemLibrary("ws2_32", .{});
        commons_mod.linkSystemLibrary("psapi", .{});
    }

    const commons = b.addLibrary(.{
        .name = "viriatum_commons",
        .root_module = commons_mod,
    });

    // --- viriatum core (static library, without main) ---

    const viriatum_mod = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    viriatum_mod.addIncludePath(viriatum_root.path(b, "src/viriatum"));
    viriatum_mod.addIncludePath(viriatum_root.path(b, "src/viriatum_commons"));
    viriatum_mod.addCSourceFiles(.{
        .root = viriatum_root.path(b, "src/viriatum"),
        .files = &viriatum_sources,
        .flags = c_flags,
    });
    if (target.result.os.tag != .windows) {
        viriatum_mod.linkSystemLibrary("dl", .{});
    }

    const viriatum = b.addLibrary(.{
        .name = "viriatum",
        .root_module = viriatum_mod,
    });
    viriatum.linkLibrary(commons);

    // --- example executable ---

    const exe_mod = b.createModule(.{
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    exe_mod.addIncludePath(viriatum_root.path(b, "src/viriatum"));
    exe_mod.addIncludePath(viriatum_root.path(b, "src/viriatum_commons"));

    // shim providing START_MEMORY and other globals normally
    // defined in viriatum.c (which we exclude to avoid main)
    exe_mod.addCSourceFile(.{
        .file = b.path("src/viriatum_shim.c"),
        .flags = c_flags,
    });

    const exe = b.addExecutable(.{
        .name = "viriatum_zig",
        .root_module = exe_mod,
    });
    exe.linkLibrary(viriatum);
    exe.linkLibrary(commons);

    b.installArtifact(exe);

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the embedded viriatum server");
    run_step.dependOn(&run_cmd.step);
}

// viriatum_commons source files (relative to src/viriatum_commons)
const commons_sources = [_][]const u8{
    "stdafx.c",
    "checksum/crc_32.c",
    "checksum/md5.c",
    "checksum/sha1.c",
    "compression/huffman.c",
    "crypto/rc4.c",
    "debug/error.c",
    "debug/logging.c",
    "encoding/base_64.c",
    "encoding/bencoding.c",
    "encoding/percent.c",
    "formats/passwd.c",
    "formats/url.c",
    "ini/ini.c",
    "io/file.c",
    "memory/memory_pool.c",
    "sorting/quicksort.c",
    "stream/bit_stream.c",
    "stream/file_stream.c",
    "stream/io_stream.c",
    "stream/memory_stream.c",
    "structures/array_list.c",
    "structures/buffer.c",
    "structures/date_time.c",
    "structures/hash_map.c",
    "structures/iterator.c",
    "structures/linked_buffer.c",
    "structures/linked_list.c",
    "structures/priority_queue.c",
    "structures/sort_map.c",
    "structures/string.c",
    "structures/string_buffer.c",
    "structures/type.c",
    "template/engine.c",
    "template/handler.c",
    "test/unit.c",
    "thread/thread_pool.c",
    "util/arguments_util.c",
};

// viriatum core source files (relative to src/viriatum)
// note: viriatum.c is excluded because it contains main(),
// we call create_service/start_service/etc. from service.c directly
const viriatum_sources = [_][]const u8{
    "stdafx.c",
    "handlers/handler_default.c",
    "handlers/handler_dispatch.c",
    "handlers/handler_file.c",
    "handlers/handler_proxy.c",
    "http/http_parser.c",
    "http/http_request.c",
    "http/http_util.c",
    "module/loading.c",
    "polling/polling_select.c",
    "stream/stream_http.c",
    "stream/stream_httpc.c",
    "stream/stream_io.c",
    "stream/stream_torrent.c",
    "system/client.c",
    "system/environment.c",
    "system/service.c",
    "system/torrent.c",
    "test/simple_test.c",
    "test/handler_file_test.c",
    "test/handler_dispatch_test.c",
    "test/test_support.c",
};
