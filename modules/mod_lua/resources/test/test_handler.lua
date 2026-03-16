-- Hive Viriatum Web Server
-- Copyright (c) 2008-2026 Hive Solutions Lda.
--
-- Minimal WSAPI handler for mod_lua test suite.
-- Returns a simple text response with predictable content
-- so that the C test harness can verify the output.

local app = {}

function app.run(wsapi_env)
    local method = wsapi_env.REQUEST_METHOD or "GET"
    local path = wsapi_env.PATH_INFO or "/"

    local body = "method=" .. method .. "\npath=" .. path .. "\n"

    local headers = {
        ["Content-Type"] = "text/plain",
        ["Content-Length"] = tostring(string.len(body)),
        ["X-Test"] = "viriatum"
    }

    return 200, headers, coroutine.wrap(function()
        coroutine.yield(body)
    end)
end

return app
