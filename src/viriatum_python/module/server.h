/*
 Hive Viriatum Web Server
 Copyright (c) 2008-2026 Hive Solutions Lda.

 This file is part of Hive Viriatum Web Server.

 Hive Viriatum Web Server is free software: you can redistribute it and/or modify
 it under the terms of the Apache License as published by the Apache
 Foundation, either version 2.0 of the License, or (at your option) any
 later version.

 Hive Viriatum Web Server is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 Apache License for more details.

 You should have received a copy of the Apache License along with
 Hive Viriatum Web Server. If not, see <http://www.apache.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#include "../stdafx.h"

/**
 * The timeout (in milliseconds) used while polling the
 * service, it bounds how long a stop request or a signal
 * may take to be noticed by the serving loop.
 */
#define VIRIATUM_PYTHON_POLL_TIMEOUT 250

/**
 * Structure describing a server object, wrapping a viriatum
 * service so that it may be driven from the interpreter.
 */
typedef struct server_python_t {
    PyObject_HEAD

    /**
     * The service that is being wrapped by the current
     * server object (owned by it).
     */
    struct service_t *service;

    /**
     * The host to which the underlying service is bound, kept
     * here as the service only retains a reference to it.
     */
    unsigned char host[VIRIATUM_MAX_HEADER_SIZE];

    /**
     * Flag controlling if the underlying service has been
     * opened and so requires a proper closing.
     */
    char opened;
} server_python;

PyObject *create_server_type_python(void);
