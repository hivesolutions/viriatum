-- Hive Viriatum Web Server
-- Copyright (c) 2008-2026 Hive Solutions Lda.
--
-- Example Lua handler for Viriatum Web Server.
-- Accessible at /lua_demo when the lua handler is configured.

function handle()
    local body =
        "            _      _       _\n" ..
        " __   ___ _(_)_ __(_) __ _| |_ _   _ _ __ ___\n" ..
        " \\ \\ / / | '_| | |/ _` | __| | | | '_ ` _ \\\n" ..
        "  \\ V /| | | | | | (_| | |_| |_| | | | | | |\n" ..
        "   \\_/ |_|_| |_|_|\\__,_|\\__|\\__,_|_| |_| |_|\n" ..
        "\n" ..
        "  engine:  viriatum (lua handler)\n" ..
        "  runtime: " .. _VERSION .. "\n" ..
        "  status:  ok\n"
    local header = "HTTP/1.1 200 OK\r\n" ..
        "Content-Type: text/plain\r\n" ..
        "Content-Length: " .. string.len(body) .. "\r\n" ..
        "Connection: keep-alive\r\n\r\n"
    local message = header .. body
    write_connection(connection, message, string.len(message))
end
