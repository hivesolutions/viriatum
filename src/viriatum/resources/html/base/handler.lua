-- Hive Viriatum Web Server
-- Copyright (c) 2008-2026 Hive Solutions Lda.
--
-- Example Lua handler for Viriatum Web Server.
-- Accessible at /lua_demo when the lua handler is configured.

function handle()
    local body = "Hello World from Viriatum Lua!\n"
    local header = "HTTP/1.1 200 OK\r\n" ..
        "Content-Type: text/plain\r\n" ..
        "Content-Length: " .. string.len(body) .. "\r\n" ..
        "Connection: keep-alive\r\n\r\n"
    local message = header .. body
    write_connection(connection, message, string.len(message))
end
