const std = @import("std");

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
extern fn print_options_service(service: *service_t) c_uint;
extern fn start_service(service: *service_t) c_uint;
extern fn create_hash_map(hash_map_pointer: **hash_map_t, initial_size: usize) void;
extern fn delete_hash_map(hash_map: *hash_map_t) void;

pub fn main() void {
    const builtin = @import("builtin");
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

    // create a service instance with a descriptive program name
    var service: *service_t = undefined;
    create_service(&service, "viriatum", "viriatum_zig");

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

    // print the service banner (version, platform, flags)
    _ = print_options_service(service);

    print("\nstarting event loop, press ctrl+c to stop...\n", .{});

    // initialize platform sockets (required on Windows)
    const ws2 = std.os.windows.ws2_32;
    var wsa_data: ws2.WSADATA = undefined;
    _ = ws2.WSAStartup(0x0202, &wsa_data);

    // start the service — this blocks in the polling loop
    // accepting connections and dispatching to the file handler
    result = start_service(service);

    // cleanup sockets
    _ = ws2.WSACleanup();

    if (result != 0) {
        print("error: service exited with error\n", .{});
    }

    // destroy the service and free all resources
    delete_service(service);
    delete_hash_map(arguments);

    print("server stopped.\n", .{});
}
