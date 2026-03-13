/*
 Hive Viriatum Modules
 Copyright (c) 2008-2026 Hive Solutions Lda.

 This file is part of Hive Viriatum Modules.

 Hive Viriatum Modules is free software: you can redistribute it and/or modify
 it under the terms of the Apache License as published by the Apache
 Foundation, either version 2.0 of the License, or (at your option) any
 later version.

 Hive Viriatum Modules is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 Apache License for more details.

 You should have received a copy of the Apache License along with
 Hive Viriatum Modules. If not, see <http://www.apache.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "extension.h"

luaL_Reg viriatum_methods[6] = {
    { "connections", lua_viriatum_connections },
    { "connections_l", lua_viriatum_connections_l },
    { "connection_info", lua_viriatum_connection_info },
    { "uptime", lua_viriatum_uptime },
    { "modules", lua_viriatum_modules },
    { NULL, NULL }
};

int lua_viriatum_connections(lua_State *lua_state) {
    lua_pushinteger(lua_state, (lua_Integer) _service->connections_list->size);
    return 1;
}

int lua_viriatum_connections_l(lua_State *lua_state) {
    /* allocates space for the local variables that are
    going to be used in the construction of the connections */
    struct iterator_t *iterator;
    struct connection_t *connection;
    unsigned long long delta;
    char uptime[128];
    char is_empty;
    int index;

    /* creates a new table to hold the connection dictionaries
    that will be returned to the Lua caller */
    lua_newtable(lua_state);
    index = 1;

    /* creates an iterator object for the current list of connections
    available in the viriatum engine */
    create_iterator_linked_list(_service->connections_list, &iterator);

    /* iterates continuously over the complete set of connections
    in the viriatum running instance */
    while(TRUE) {
        /* retrieves the next connection value from the iterator
        and verifies if its value is defined in case it's not this
        is the end of iteration and so the cycle must be break */
        get_next_iterator(iterator, (void **) &connection);
        if(connection == NULL) { break; }

        /* retrieves the delta value by calculating the difference between
        the current time and the creation time then uses it to calculate
        the uptime for the connection as a string description */
        delta = (unsigned long long) time(NULL) - connection->creation;
        format_delta(uptime, sizeof(uptime), delta, 2);

        /* verifies if the current host is empty, this is a special
        case where no resolution of the value was possible */
        is_empty = connection->host[0] == '\0';

        /* creates a table with the connection attributes and
        sets it in the result table at the current index */
        lua_newtable(lua_state);

        lua_pushstring(lua_state, is_empty ? "N/A" : (char *) connection->host);
        lua_setfield(lua_state, -2, "host");

        lua_pushinteger(lua_state, (lua_Integer) connection->id);
        lua_setfield(lua_state, -2, "id");

        lua_pushstring(lua_state, uptime);
        lua_setfield(lua_state, -2, "uptime");

        lua_rawseti(lua_state, -2, index);
        index++;
    }

    /* deletes the iterator for the connections list in order to
    avoid any memory leak that could arise from this */
    delete_iterator_linked_list(_service->connections_list, iterator);

    return 1;
}

int lua_viriatum_connection_info(lua_State *lua_state) {
    /* allocates space for the local variables that are
    going to be used in the construction of the connection info */
    long id;
    char is_empty;
    struct iterator_t *iterator;
    struct connection_t *connection;
    unsigned long long delta;
    char uptime[128];

    /* retrieves the id argument from the Lua call */
    id = (long) luaL_checkinteger(lua_state, 1);

    /* creates an iterator object for the current list of connections
    available in the viriatum engine */
    create_iterator_linked_list(_service->connections_list, &iterator);

    /* iterates continuously over the complete set of connections
    searching for the one with the matching identifier */
    while(TRUE) {
        get_next_iterator(iterator, (void **) &connection);
        if(connection == NULL) { break; }
        if((long) connection->id == id) { break; }
    }

    /* deletes the iterator for the connections list in order to
    avoid any memory leak that could arise from this */
    delete_iterator_linked_list(_service->connections_list, iterator);

    /* in case no connection was found returns nil to indicate
    that the requested connection does not exist */
    if(connection == NULL || (long) connection->id != id) {
        lua_pushnil(lua_state);
        return 1;
    }

    /* retrieves the delta value by calculating the difference between
    the current time and the creation time then uses it to calculate
    the uptime for the connection as a string description */
    delta = (unsigned long long) time(NULL) - connection->creation;
    format_delta(uptime, sizeof(uptime), delta, 2);

    /* verifies if the current host is empty, this is a special
    case where no resolution of the value was possible */
    is_empty = connection->host[0] == '\0';

    /* creates a table with the connection attributes and
    returns it to the Lua caller */
    lua_newtable(lua_state);

    lua_pushstring(lua_state, is_empty ? "N/A" : (char *) connection->host);
    lua_setfield(lua_state, -2, "host");

    lua_pushinteger(lua_state, (lua_Integer) connection->id);
    lua_setfield(lua_state, -2, "id");

    lua_pushstring(lua_state, uptime);
    lua_setfield(lua_state, -2, "uptime");

    return 1;
}

int lua_viriatum_uptime(lua_State *lua_state) {
    lua_pushstring(lua_state, _service->get_uptime(_service, 2));
    return 1;
}

int lua_viriatum_modules(lua_State *lua_state) {
    lua_pushstring(lua_state, (char *) _service->modules);
    return 1;
}

int lua_input_new(lua_State *lua_state) {
    /* creates a new input userdata with zero size and position,
    this is the default constructor for the input type */
    lua_input_push(lua_state, NULL, 0);
    return 1;
}

int lua_input_read(lua_State *lua_state) {
    /* retrieves the input userdata from the first argument */
    struct lua_input_t *input = (struct lua_input_t *) luaL_checkudata(lua_state, 1, VIRIATUM_LUA_INPUT_NAME);

    /* calculates the remaining bytes to be read from the
    current position in the post data buffer */
    size_t remaining = input->size - input->position;

    /* pushes the remaining data as a Lua string and then
    advances the position to the end of the buffer */
    lua_pushlstring(lua_state, (char *) &input->post_data[input->position], remaining);
    input->position += remaining;

    return 1;
}

int lua_input_close(lua_State *lua_state) {
    return 0;
}

int lua_input_readline(lua_State *lua_state) {
    lua_pushnil(lua_state);
    return 1;
}

int lua_input_readlines(lua_State *lua_state) {
    lua_pushnil(lua_state);
    return 1;
}

int lua_input_gc(lua_State *lua_state) {
    return 0;
}

int lua_input_tostring(lua_State *lua_state) {
    lua_pushstring(lua_state, "viriatum.input");
    return 1;
}

void lua_input_push(lua_State *lua_state, unsigned char *post_data, size_t size) {
    /* allocates space for the input userdata in the Lua state
    and populates it with the provided buffer reference */
    struct lua_input_t *input = (struct lua_input_t *) lua_newuserdata(lua_state, sizeof(struct lua_input_t));
    input->post_data = post_data;
    input->position = 0;
    input->size = size;

    /* sets the metatable for the input userdata so that
    the Lua runtime can resolve method calls on it */
    luaL_getmetatable(lua_state, VIRIATUM_LUA_INPUT_NAME);
    lua_setmetatable(lua_state, -2);
}

int lua_error_write(lua_State *lua_state) {
    /* retrieves the message argument from the Lua call */
    const char *message = luaL_checkstring(lua_state, 1);

    /* writes the error message to the viriatum log using the
    error context macro for visibility in the server log */
    V_ERROR_CTX_F("mod_lua", "%s\n", message);

    return 0;
}
