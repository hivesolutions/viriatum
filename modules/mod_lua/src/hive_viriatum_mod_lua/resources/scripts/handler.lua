-- Hive Viriatum Modules
-- Copyright (C) 2008 Hive Solutions Lda.
--
-- This file is part of Hive Viriatum Modules.
--
-- Hive Viriatum Modules is free software: you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- Hive Viriatum Modules is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with Hive Viriatum Modules. If not, see <http://www.gnu.org/licenses/>.
--
-- __author__    = João Magalhães <joamag@hive.pt>
-- __version__   = 1.0.0
-- __revision__  = $LastChangedRevision$
-- __date__      = $LastChangedDate$
-- __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
-- __license__   = GNU General Public License (GPL), Version 3

files = {}

function write_stream(contents)
    local contents_length = contents:len()
    local message = string.format("HTTP/1.1 200 OK\r\nServer: Lua Bindings\r\nConnection: Keep-Alive\r\nContent-Length: %d\r\n\r\n%s", contents_length, contents)
    local message_length = message:len()
    write_connection(connection, message, message_length)
end

function write_file(path)
    -- retrieves the contents from the (file) cache
    -- this may create a miss in case the file
    -- is not already loaded
    local contents = files[path]

    if contents == nil then
        local file = io.open(path, "rb")
        contents = file:read("*all")
        io.close(file)
        files[path] = contents
    end

    write_stream(contents)
end

function handle()
    --write_file("handler.lua")
    write_stream("Hello World")
end
