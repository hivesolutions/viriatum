-- Hive Viriatum Web Server
-- Copyright (c) 2008-2026 Hive Solutions Lda.
--
-- Example Lua handler for Viriatum Web Server.
-- Accessible at /lua_demo when the lua handler is configured.

local viriatum = require("viriatum")

local app = {}

function app.run(wsapi_env)
    local body =
        "            _      _       _\n" ..
        " __   ___ _(_)_ __(_) __ _| |_ _   _ _ __ ___\n" ..
        " \\ \\ / / | '_| | |/ _` | __| | | | '_ ` _ \\\n" ..
        "  \\ V /| | | | | | (_| | |_| |_| | | | | | |\n" ..
        "   \\_/ |_|_| |_|_|\\__,_|\\__|\\__,_|_| |_| |_|\n" ..
        "\n" ..
        "  engine:      " .. viriatum.NAME .. " (lua handler)\n" ..
        "  version:     " .. viriatum.VERSION .. "\n" ..
        "  platform:    " .. viriatum.PLATFORM .. "\n" ..
        "  description: " .. viriatum.DESCRIPTION .. "\n" ..
        "\n" ..
        "  uptime:      " .. viriatum.uptime() .. "\n" ..
        "  connections: " .. viriatum.connections() .. "\n" ..
        "  method:      " .. (wsapi_env.REQUEST_METHOD or "?") .. " " .. (wsapi_env.PATH_INFO or "/") .. "\n" ..
        "\n" ..
        "  runtime:     " .. _VERSION .. "\n" ..
        "  compiler:    " .. viriatum.COMPILER .. " / " .. viriatum.COMPILER_VERSION .. "\n" ..
        "  compiled:    " .. viriatum.COMPILATION_DATE .. " @ " .. viriatum.COMPILATION_TIME .. "\n" ..
        "  flags:       " .. viriatum.COMPILATION_FLAGS .. "\n" ..
        "  modules:     " .. viriatum.modules() .. "\n" ..
        "  copyright:   " .. viriatum.COPYRIGHT .. "\n"

    local headers = {
        ["Content-Type"] = "text/plain",
        ["Content-Length"] = tostring(string.len(body))
    }

    return 200, headers, coroutine.wrap(function()
        coroutine.yield(body)
    end)
end

return app
