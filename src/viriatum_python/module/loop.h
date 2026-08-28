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
 * The timeout (in milliseconds) used while polling the service
 * whenever at least one task is still pending, it bounds the
 * resolution of the timers scheduled by the applications.
 */
#define VIRIATUM_ASGI_POLL_TIMEOUT 5

/**
 * The number of iterations that the draining of the cancelled
 * tasks may take before the loop is closed anyway.
 */
#define VIRIATUM_ASGI_DRAIN_ITERATIONS 64

/**
 * Structure wrapping the asyncio event loop that is driven by
 * the serving loop, one of these exists per server.
 */
typedef struct loop_python_t {
    /**
     * The asyncio module, kept so that the various helper
     * functions of it remain reachable.
     */
    PyObject *module;

    /**
     * The event loop that is going to be advanced once per
     * iteration of the serving loop (owned by the structure).
     */
    PyObject *loop;
} loop_python;

ERROR_CODE create_loop_python(struct loop_python_t **loop_python_pointer);
ERROR_CODE delete_loop_python(struct loop_python_t *loop_python);
ERROR_CODE run_once_loop_python(struct loop_python_t *loop_python);
ERROR_CODE run_slice_loop_python(struct loop_python_t *loop_python, double timeout);
double time_loop_python(struct loop_python_t *loop_python);
ERROR_CODE attach_loop_python(struct loop_python_t *loop_python);
size_t pending_loop_python(struct loop_python_t *loop_python);
PyObject *create_task_loop_python(struct loop_python_t *loop_python, PyObject *coroutine);
PyObject *create_future_loop_python(struct loop_python_t *loop_python);
ERROR_CODE resolve_future_loop_python(PyObject *future, PyObject *result);
