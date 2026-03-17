const std = @import("std");
const builtin = @import("builtin");

const print = std.debug.print;

// declare only the C functions and types we need, avoiding
// @cImport which chokes on circular type references in the
// viriatum headers
const service_t = opaque {};
const hash_map_t = opaque {};

extern fn create_service(
    service_pointer: **service_t,
    name: [*]const u8,
    program_name: [*]const u8,
) void;
extern fn delete_service(service: *service_t) void;
extern fn load_specifications(service: *service_t) c_uint;
extern fn load_options_service(service: *service_t, arguments: *hash_map_t) c_uint;
extern fn calculate_options_service(service: *service_t) c_uint;
extern fn calculate_locations_service(service: *service_t) c_uint;
extern fn print_options_service(service: *service_t) c_uint;
extern fn start_service(service: *service_t) c_uint;
extern fn create_hash_map(hash_map_pointer: **hash_map_t, initial_size: usize) void;
extern fn delete_hash_map(hash_map: *hash_map_t) void;

pub fn main() void {
    const zig_version = builtin.zig_version;
    const os_tag = @tagName(builtin.os.tag);
    print("viriatum/zig - embedded server example (zig {d}.{d}.{d}, {s})\n\n", .{
        zig_version.major,
        zig_version.minor,
        zig_version.patch,
        os_tag,
    });

    // create an empty arguments map (uses all defaults)
    var arguments: *hash_map_t = undefined;
    create_hash_map(&arguments, 0);
    defer delete_hash_map(arguments);

    // create a service instance with a descriptive program name
    var service: *service_t = undefined;
    create_service(&service, "viriatum", "viriatum_zig");
    defer delete_service(service);

    // load compile-time specifications (version, platform, etc.)
    var result = load_specifications(service);
    if (result != 0) {
        print("error: failed to load specifications\n", .{});
        return;
    }

    // load default options (port, host, handler, etc.)
    result = load_options_service(service, arguments);
    if (result != 0) {
        print("error: failed to load options\n", .{});
        return;
    }

    // calculate derived options from the loaded values
    result = calculate_options_service(service);
    if (result != 0) {
        print("error: failed to calculate options\n", .{});
        return;
    }

    // calculates the locations structure for the service based
    // on the currently loaded configuration
    _ = calculate_locations_service(service);

    // print the service banner (version, platform, flags)
    _ = print_options_service(service);

    print("\nstarting event loop, press ctrl+c to stop...\n", .{});

    // initialize platform sockets (required on Windows,
    // no-op on Unix where sockets work out of the box)
    if (comptime builtin.os.tag == .windows) {
        const ws2 = std.os.windows.ws2_32;
        var wsa_data: ws2.WSADATA = undefined;
        _ = ws2.WSAStartup(0x0202, &wsa_data);
    }
    defer {
        if (comptime builtin.os.tag == .windows) {
            _ = std.os.windows.ws2_32.WSACleanup();
        }
    }

    // start the service - this blocks in the polling loop
    // accepting connections and dispatching requests
    result = start_service(service);

    if (result != 0) {
        print("error: service exited with error\n", .{});
    }

    print("server stopped.\n", .{});
}
