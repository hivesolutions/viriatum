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

#include "stdafx.h"

#include "loop.h"

ERROR_CODE create_loop_python(struct loop_python_t **loop_python_pointer) {
    /* retrieves the structure size and allocates space for it, then
    resets the complete set of values so that no invalid reference
    is kept in the newly created structure */
    size_t loop_size = sizeof(struct loop_python_t);
    struct loop_python_t *loop_python =
        (struct loop_python_t *) MALLOC(loop_size);
    memset(loop_python, 0, loop_size);

    /* imports the asyncio module, it provides both the creation of
    the event loop and the introspection of the pending tasks */
    loop_python->module = PyImport_ImportModule("asyncio");
    if(loop_python->module == NULL) {
        FREE(loop_python);
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Problem importing the asyncio module"
        );
    }

    /* creates the event loop that is going to be driven by the serving
    loop and sets it as the current one, so that the application is
    able to reach it through the usual accessors */
    loop_python->loop = PyObject_CallMethod(loop_python->module, "new_event_loop", NULL);
    if(loop_python->loop == NULL) {
        Py_DECREF(loop_python->module);
        FREE(loop_python);
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Problem creating the asyncio event loop"
        );
    }
    /* sets the structure in the provided pointer */
    *loop_python_pointer = loop_python;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_loop_python(struct loop_python_t *loop_python) {
    /* allocates space for the sequence of tasks that are still
    pending and for the index used in the iteration over it */
    PyObject *tasks;
    PyObject *task;
    Py_ssize_t index;
    Py_ssize_t count;

    /* cancels the complete set of tasks that are still pending, none
    of them is ever going to be advanced once the loop is closed */
    if(loop_python->loop != NULL) {
        tasks = PyObject_CallMethod(loop_python->module, "all_tasks", "O", loop_python->loop);
        if(tasks == NULL) { PyErr_Clear(); }
        else {
            tasks = PySequence_Fast(tasks, "tasks must be a sequence");
            count = tasks == NULL ? 0 : PySequence_Fast_GET_SIZE(tasks);
            for(index = 0; index < count; index++) {
                task = PySequence_Fast_GET_ITEM(tasks, index);
                Py_XDECREF(PyObject_CallMethod(task, "cancel", NULL));
            }
            Py_XDECREF(tasks);
            PyErr_Clear();
        }

        /* advances the loop one last time so that the cancellation of
        the various tasks is properly propagated to them */
        run_once_loop_python(loop_python);
        Py_XDECREF(PyObject_CallMethod(loop_python->loop, "close", NULL));
        PyErr_Clear();
        Py_DECREF(loop_python->loop);
        loop_python->loop = NULL;
    }

    /* releases the reference held on the asyncio module and then
    releases the structure itself */
    if(loop_python->module != NULL) {
        Py_DECREF(loop_python->module);
        loop_python->module = NULL;
    }
    FREE(loop_python);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE attach_loop_python(struct loop_python_t *loop_python) {
    /* sets the loop as the current one of the calling thread, this is
    what makes the accessors of the asyncio module resolve to it for
    the application code that runs outside of a coroutine, note that
    it must be called from the thread that advances the loop */
    Py_XDECREF(PyObject_CallMethod(
        loop_python->module,
        "set_event_loop",
        "O",
        loop_python->loop
    ));
    PyErr_Clear();

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE run_slice_loop_python(struct loop_python_t *loop_python, double timeout) {
    /* allocates space for both the coroutine that bounds the slice
    and for the result of the running of the loop */
    PyObject *sleep;
    PyObject *result;

    /* creates the coroutine that sleeps for the duration of the slice
    and runs the loop until it completes, every other task pending in
    the loop is advanced meanwhile, this is the portable way of
    letting real time pass as the various loop implementations differ
    in how they behave around a plain stopping callback */
    sleep = PyObject_CallMethod(loop_python->module, "sleep", "d", timeout);
    if(sleep == NULL) {
        PyErr_Clear();
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Problem creating the sleep coroutine"
        );
    }
    result = PyObject_CallMethod(loop_python->loop, "run_until_complete", "O", sleep);
    Py_DECREF(sleep);
    if(result == NULL) {
        PyErr_Clear();
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Problem running the event loop"
        );
    }
    Py_DECREF(result);

    /* raises no error */
    RAISE_NO_ERROR;
}

double time_loop_python(struct loop_python_t *loop_python) {
    /* retrieves the monotonic clock of the loop, it is the one that
    the timers scheduled in it are measured against */
    PyObject *result = PyObject_CallMethod(loop_python->loop, "time", NULL);
    double value;
    if(result == NULL) { PyErr_Clear(); return 0.0; }
    value = PyFloat_AsDouble(result);
    Py_DECREF(result);
    if(PyErr_Occurred()) { PyErr_Clear(); return 0.0; }
    return value;
}

ERROR_CODE run_once_loop_python(struct loop_python_t *loop_python) {
    /* allocates space for the bound stop method of the loop and for
    the result of the various calls performed over it */
    PyObject *stop;
    PyObject *result;

    /* schedules the stopping of the loop as the first of the ready
    callbacks, this is what makes the run below advance the loop by
    exactly one iteration instead of blocking on its own selector */
    stop = PyObject_GetAttrString(loop_python->loop, "stop");
    if(stop == NULL) { PyErr_Clear(); RAISE_NO_ERROR; }
    result = PyObject_CallMethod(loop_python->loop, "call_soon", "O", stop);
    Py_DECREF(stop);
    if(result == NULL) { PyErr_Clear(); RAISE_NO_ERROR; }
    Py_DECREF(result);

    /* runs the loop until the stopping callback above is reached, no
    blocking happens as the ready queue is never empty */
    result = PyObject_CallMethod(loop_python->loop, "run_forever", NULL);
    if(result == NULL) { PyErr_Clear(); } else { Py_DECREF(result); }

    /* raises no error */
    RAISE_NO_ERROR;
}

size_t pending_loop_python(struct loop_python_t *loop_python) {
    /* allocates space for the sequence of tasks that are still
    pending and for the resulting count of them */
    PyObject *tasks;
    Py_ssize_t count;

    /* retrieves the complete set of tasks that are still pending in
    the loop, the done ones are never part of the resulting set */
    tasks = PyObject_CallMethod(loop_python->module, "all_tasks", "O", loop_python->loop);
    if(tasks == NULL) { PyErr_Clear(); return 0; }
    count = PyObject_Length(tasks);
    Py_DECREF(tasks);
    if(count < 0) { PyErr_Clear(); return 0; }

    /* returns the number of tasks that are still pending */
    return (size_t) count;
}

PyObject *create_task_loop_python(struct loop_python_t *loop_python, PyObject *coroutine) {
    /* creates the task wrapping the provided coroutine, from this
    point on it is advanced by the iterations of the serving loop */
    return PyObject_CallMethod(loop_python->loop, "create_task", "O", coroutine);
}

PyObject *create_future_loop_python(struct loop_python_t *loop_python) {
    /* creates a future bound to the loop, these are the values that
    are returned by both the receive and the send callables */
    return PyObject_CallMethod(loop_python->loop, "create_future", NULL);
}

ERROR_CODE resolve_future_loop_python(PyObject *future, PyObject *result) {
    /* allocates space for both the flag controlling if the future has
    already been resolved and for the result of the resolution */
    PyObject *done;
    PyObject *resolved;
    int is_done;

    /* in case the future has already been resolved, which happens when
    the associated task has been cancelled in the meantime, nothing
    may be set on it (that would raise an invalid state error) */
    done = PyObject_CallMethod(future, "done", NULL);
    if(done == NULL) { PyErr_Clear(); RAISE_NO_ERROR; }
    is_done = PyObject_IsTrue(done);
    Py_DECREF(done);
    if(is_done != 0) { RAISE_NO_ERROR; }

    /* resolves the future with the provided result, waking up the task
    that is waiting on it on the next iteration of the loop */
    resolved = PyObject_CallMethod(future, "set_result", "O", result);
    if(resolved == NULL) { PyErr_Clear(); } else { Py_DECREF(resolved); }

    /* raises no error */
    RAISE_NO_ERROR;
}
