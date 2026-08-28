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

#include "server.h"
#include "handler.h"
#include "handler_asgi.h"

static char _has_marker_server_python(PyObject *application, const char *name) {
    /* verifies if the application carries the provided marker, they
    are the ones set by the adaptation helpers of asgiref */
    PyObject *marker = PyObject_GetAttrString(application, name);
    int is_set;
    if(marker == NULL) { PyErr_Clear(); return FALSE; }
    is_set = PyObject_IsTrue(marker);
    Py_DECREF(marker);
    return is_set != 0 ? TRUE : FALSE;
}

static char _is_asgi_server_python(PyObject *application) {
    /* allocates space for the various objects used during the
    inspection of the application that has been provided */
    PyObject *module;
    PyObject *result;
    PyObject *call;
    int is_coroutine = 0;

    /* an application that carries either of the markers is an asgi one
    regardless of its shape, a wsgi one never carries them */
    if(_has_marker_server_python(application, "_asgi_single_callable") == TRUE) {
        return TRUE;
    }
    if(_has_marker_server_python(application, "_asgi_double_callable") == TRUE) {
        return TRUE;
    }

    /* imports the inspect module, it provides the detection of the
    coroutine functions that is required for the interface */
    module = PyImport_ImportModule("inspect");
    if(module == NULL) { PyErr_Clear(); return FALSE; }

    /* verifies if the application is itself a coroutine function, the
    usual shape of an asgi application defined as a plain function */
    result = PyObject_CallMethod(module, "iscoroutinefunction", "O", application);
    if(result == NULL) { PyErr_Clear(); }
    else {
        is_coroutine = PyObject_IsTrue(result);
        Py_DECREF(result);
    }

    /* verifies if the call method of the application is a coroutine
    one, the shape of an application defined as a class instance */
    if(is_coroutine == 0) {
        call = PyObject_GetAttrString(application, "__call__");
        if(call == NULL) { PyErr_Clear(); }
        else {
            result = PyObject_CallMethod(module, "iscoroutinefunction", "O", call);
            Py_DECREF(call);
            if(result == NULL) { PyErr_Clear(); }
            else {
                is_coroutine = PyObject_IsTrue(result);
                Py_DECREF(result);
            }
        }
    }

    /* releases the reference to the module and returns the resulting
    classification of the provided application */
    Py_DECREF(module);
    return is_coroutine != 0 ? TRUE : FALSE;
}

static char _is_double_callable_server_python(PyObject *application) {
    /* allocates space for the various objects used during the
    inspection of the application that has been provided */
    PyObject *module;
    PyObject *result;
    PyObject *call;
    int is_double = 0;
    int is_class = 0;

    /* the markers set by the adaptation helpers of asgiref take
    precedence over any inspection of the application */
    if(_has_marker_server_python(application, "_asgi_single_callable") == TRUE) {
        return FALSE;
    }
    if(_has_marker_server_python(application, "_asgi_double_callable") == TRUE) {
        return TRUE;
    }

    /* imports the inspect module, it provides both the detection of
    the classes and the one of the coroutine functions */
    module = PyImport_ImportModule("inspect");
    if(module == NULL) { PyErr_Clear(); return FALSE; }

    /* a class that has not been instantiated is a double callable one,
    the instance of it is what takes the pair of callables */
    result = PyObject_CallMethod(module, "isclass", "O", application);
    if(result == NULL) { PyErr_Clear(); }
    else {
        is_class = PyObject_IsTrue(result);
        Py_DECREF(result);
    }
    if(is_class != 0) { Py_DECREF(module); return TRUE; }

    /* an instance whose call method is a coroutine one is a single
    callable application, the shape of the third version */
    call = PyObject_GetAttrString(application, "__call__");
    if(call == NULL) { PyErr_Clear(); }
    else {
        result = PyObject_CallMethod(module, "iscoroutinefunction", "O", call);
        Py_DECREF(call);
        if(result == NULL) { PyErr_Clear(); }
        else {
            is_double = PyObject_IsTrue(result);
            Py_DECREF(result);
            if(is_double != 0) { Py_DECREF(module); return FALSE; }
        }
    }

    /* everything that is not a coroutine function of its own is taken
    as a double callable application (the legacy shape) */
    result = PyObject_CallMethod(module, "iscoroutinefunction", "O", application);
    Py_DECREF(module);
    if(result == NULL) { PyErr_Clear(); return TRUE; }
    is_double = PyObject_IsTrue(result);
    Py_DECREF(result);
    return is_double != 0 ? FALSE : TRUE;
}

static int _init_server_python(PyObject *self, PyObject *args, PyObject *kwargs) {
    /* allocates space for the various arguments that may be provided
    for the construction of the server object */
    PyObject *application;
    const char *host = VIRIATUM_DEFAULT_HOST;
    const char *www_root = NULL;
    const char *interface = VIRIATUM_PYTHON_INTERFACE_AUTO;
    int port = VIRIATUM_DEFAULT_PORT;
    char asgi;

    /* allocates space for the arguments map that is required by the
    default options loading operation */
    struct hash_map_t *arguments;
    struct service_t *service;

    /* defines the complete set of keywords accepted by the constructor
    of the server object (application is the only mandatory one) */
    static char *keywords[] = {
        "application", "host", "port", "www_root", "interface", NULL
    };

    /* retrieves the reference to the server object that is currently
    being initialized (target of the operation) */
    struct server_python_t *server_python = (struct server_python_t *) self;

    /* parses the arguments provided to the constructor according to
    the keywords sequence defined above */
    if(!PyArg_ParseTupleAndKeywords(
        args, kwargs, "O|sizs", keywords,
        &application, &host, &port, &www_root, &interface
    )) { return -1; }

    /* in case the server has already been initialized rejects the new
    initialization, otherwise the previously created service and the
    reference to its application would be leaked */
    if(server_python->service != NULL) {
        PyErr_SetString(PyExc_RuntimeError, "server is already initialized");
        return -1;
    }

    /* verifies that the provided application is a callable object as
    required by both the WSGI and the ASGI specifications */
    if(!PyCallable_Check(application)) {
        PyErr_SetString(PyExc_TypeError, "application must be callable");
        return -1;
    }

    /* resolves the interface that is going to be used for the calling
    of the application, the automatic one only tells the single
    callable applications apart from the wsgi ones, as a double
    callable one is indistinguishable from a wsgi callable */
    if(strcmp(interface, VIRIATUM_PYTHON_INTERFACE_AUTO) == 0) {
        asgi = _is_asgi_server_python(application);
        server_python->double_callable = asgi == TRUE ?
            _is_double_callable_server_python(application) : FALSE;
    } else if(strcmp(interface, VIRIATUM_PYTHON_INTERFACE_ASGI) == 0) {
        asgi = TRUE;
        server_python->double_callable = _is_double_callable_server_python(application);
    } else if(strcmp(interface, VIRIATUM_PYTHON_INTERFACE_ASGI3) == 0) {
        asgi = TRUE;
        server_python->double_callable = FALSE;
    } else if(strcmp(interface, VIRIATUM_PYTHON_INTERFACE_ASGI2) == 0) {
        asgi = TRUE;
        server_python->double_callable = TRUE;
    } else if(strcmp(interface, VIRIATUM_PYTHON_INTERFACE_WSGI) == 0) {
        asgi = FALSE;
        server_python->double_callable = FALSE;
    } else {
        PyErr_SetString(
            PyExc_ValueError,
            "interface must be auto, wsgi, asgi, asgi2 or asgi3"
        );
        return -1;
    }

    /* verifies that the port is contained in the range that may be
    represented by the underlying value, as it is narrowed below */
    if(port < 0 || port > 65535) {
        PyErr_SetString(PyExc_ValueError, "port must be between 0 and 65535");
        return -1;
    }

    /* verifies that both the host and the www root fit the buffers that
    are going to receive them, avoiding an overflow of either of them */
    if(strlen(host) >= VIRIATUM_MAX_HEADER_SIZE) {
        PyErr_SetString(PyExc_ValueError, "host is too long");
        return -1;
    }
    if(www_root != NULL && strlen(www_root) >= VIRIATUM_MAX_PATH_SIZE) {
        PyErr_SetString(PyExc_ValueError, "www_root is too long");
        return -1;
    }

    /* creates the service and loads both the specifications and the
    default options into it, note that no configuration file is read
    so that the server is completely defined by the arguments */
    create_service(
        &service,
        (unsigned char *) VIRIATUM_NAME,
        (unsigned char *) VIRIATUM_NAME
    );
    load_specifications(service);
    create_hash_map(&arguments, 0);
    _default_options_service(service, arguments);
    delete_hash_map(arguments);

    /* sets the various options according to the provided arguments,
    the modules are not loaded as they would re initialize the
    interpreter that is currently hosting the server */
    service->options->port = (unsigned short) port;
    SPRINTF((char *) server_python->host, VIRIATUM_MAX_HEADER_SIZE, "%s", host);
    service->options->address = server_python->host;
    service->options->handler_name = asgi == TRUE ?
        VIRIATUM_ASGI_HANDLER_NAME : VIRIATUM_PYTHON_HANDLER_NAME;
    service->options->load_modules = 0;
    service->options->workers = 0;
    service->options->ip6 = 0;
    if(www_root != NULL) {
        SPRINTF((char *) service->options->www_root, VIRIATUM_MAX_PATH_SIZE, "%s", www_root);
    }

    /* calculates both the options and the locations of the service so
    that the derived values are properly resolved */
    calculate_options_service(service);
    calculate_locations_service(service);

    /* registers the handler of the resolved interface in the service,
    this operation takes a reference on the provided application, the
    asgi one requires an event loop to be created for it */
    if(asgi == TRUE) {
        if(IS_ERROR_CODE(create_loop_python(&server_python->loop_python))) {
            delete_service(service);
            PyErr_SetString(PyExc_RuntimeError, (char *) GET_ERROR());
            return -1;
        }
        register_handler_asgi(
            service,
            application,
            server_python->loop_python,
            server_python->double_callable
        );
    } else {
        register_handler_python(service, application);
    }

    /* sets the service in the server object and marks it as not
    opened, the opening happens on the serving operation */
    server_python->service = service;
    server_python->opened = FALSE;

    /* returns the success value as the initialization of the
    server object went without problems */
    return 0;
}

static void _dealloc_server_python(PyObject *self) {
    /* retrieves both the type of the object, required for the releasing
    of the reference that every instance holds on it, and the reference
    to the server object itself */
    PyTypeObject *type = Py_TYPE(self);
    struct server_python_t *server_python = (struct server_python_t *) self;
    if(server_python->service != NULL) {
        if(server_python->opened == TRUE) {
            close_service(server_python->service);
            server_python->opened = FALSE;
        }
        if(server_python->loop_python == NULL) {
            unregister_handler_python(server_python->service);
        } else {
            unregister_handler_asgi(server_python->service);
        }
        delete_service(server_python->service);
        server_python->service = NULL;
    }

    /* releases the event loop, cancelling every task that may still
    be pending in it (only set for the asgi applications) */
    if(server_python->loop_python != NULL) {
        delete_loop_python(server_python->loop_python);
        server_python->loop_python = NULL;
    }

    /* releases the object itself and then the reference that it was
    holding on its own (heap allocated) type */
    type->tp_free(self);
    Py_DECREF(type);
}

static PyObject *_serve_forever_server_python(PyObject *self, PyObject *args) {
    /* retrieves the reference to the server object and the service
    that is going to be driven by the current loop */
    struct server_python_t *server_python = (struct server_python_t *) self;
    struct service_t *service = server_python->service;

    /* opens the service, this creates the sockets and bootstraps the
    complete set of structures required by the polling operations */
    if(server_python->opened == FALSE) {
        if(IS_ERROR_CODE(open_service(service))) {
            PyErr_SetString(PyExc_RuntimeError, (char *) GET_ERROR());
            return NULL;
        }
        server_python->opened = TRUE;

        /* bounds the polling timeout so that the loop comes back on a
        regular basis, this is what allows both the signal checking and
        the stopping of the service to be noticed in a timely manner */
        service->polling->timeout = VIRIATUM_PYTHON_POLL_TIMEOUT;

        /* attaches the loop to the current thread, the one that is
        going to advance it, so that the accessors of the asyncio
        module resolve to it for the application code */
        if(server_python->loop_python != NULL) {
            attach_loop_python(server_python->loop_python);
        }

        /* runs the startup event of the lifespan protocol, a failure of
        it aborts the serving as the application refused to boot */
        if(server_python->loop_python != NULL &&
            IS_ERROR_CODE(startup_handler_asgi(service))) {
            close_service(service);
            server_python->opened = FALSE;
            PyErr_SetString(PyExc_RuntimeError, (char *) GET_ERROR());
            return NULL;
        }
    }

    /* iterates continuously while the service is open, the polling
    operation is the only one that may block and so it's the only one
    that runs with the global interpreter lock released */
    while(service->status == STATUS_OPEN) {
        Py_BEGIN_ALLOW_THREADS
        poll_service(service);
        Py_END_ALLOW_THREADS
        call_service(service);

        /* advances the event loop by a single iteration, this is what
        makes the tasks running the various applications progress, the
        polling timeout is shortened while any of them is pending so
        that the timers they schedule keep a usable resolution */
        if(server_python->loop_python != NULL) {
            run_once_loop_python(server_python->loop_python);
            service->polling->timeout =
                pending_loop_python(server_python->loop_python) > 0 ?
                VIRIATUM_ASGI_POLL_TIMEOUT : VIRIATUM_PYTHON_POLL_TIMEOUT;
        }

        /* verifies if a signal has been raised in the meantime, this
        is what makes the keyboard interrupt work as expected */
        if(PyErr_CheckSignals() != 0) {
            stop_service(service);
            if(server_python->loop_python != NULL) { shutdown_handler_asgi(service); }
            close_service(service);
            server_python->opened = FALSE;
            return NULL;
        }
    }

    /* runs the shutdown event of the lifespan protocol and then closes
    the service releasing every structure that has been created
    during the opening of it */
    if(server_python->loop_python != NULL) { shutdown_handler_asgi(service); }
    close_service(service);
    server_python->opened = FALSE;

    /* returns the none value as the serving operation finished
    without any kind of problem */
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *_stop_server_python(PyObject *self, PyObject *args) {
    /* retrieves the reference to the server object and stops the
    associated service, the loop notices it on the next iteration */
    struct server_python_t *server_python = (struct server_python_t *) self;
    stop_service(server_python->service);

    /* returns the none value */
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *_connections_server_python(PyObject *self, void *closure) {
    /* retrieves the reference to the server object and uses the
    connections list of the service to count them */
    struct server_python_t *server_python = (struct server_python_t *) self;
    return PyLong_FromSize_t(server_python->service->connections_list->size);
}

static PyObject *_asgi_server_python(PyObject *self, void *closure) {
    /* retrieves the reference to the server object, only the asgi
    applications have an event loop created for them */
    struct server_python_t *server_python = (struct server_python_t *) self;
    return PyBool_FromLong(server_python->loop_python == NULL ? 0 : 1);
}

static PyObject *_double_callable_server_python(PyObject *self, void *closure) {
    /* retrieves the reference to the server object, the flag is only
    ever set for the applications of the legacy interface */
    struct server_python_t *server_python = (struct server_python_t *) self;
    return PyBool_FromLong(server_python->double_callable == TRUE ? 1 : 0);
}

static PyObject *_uptime_server_python(PyObject *self, void *closure) {
    /* retrieves the reference to the server object and uses the
    service to retrieve the textual uptime representation */
    struct server_python_t *server_python = (struct server_python_t *) self;
    struct service_t *service = server_python->service;
    return PyUnicode_FromString((char *) service->get_uptime(service, 2));
}

static PyMethodDef server_methods[] = {
    {"serve_forever", _serve_forever_server_python, METH_NOARGS, "Runs the server loop until stopped."},
    {"stop", _stop_server_python, METH_NOARGS, "Stops the server loop."},
    {NULL, NULL, 0, NULL}
};

static PyGetSetDef server_getset[] = {
    {"asgi", _asgi_server_python, NULL, "If the application is served through the asgi interface.", NULL},
    {"double_callable", _double_callable_server_python, NULL, "If the application is called through the legacy asgi interface.", NULL},
    {"connections", _connections_server_python, NULL, "The number of currently open connections.", NULL},
    {"uptime", _uptime_server_python, NULL, "The uptime of the server as a string.", NULL},
    {NULL, NULL, NULL, NULL, NULL}
};

static PyType_Slot server_slots[] = {
    {Py_tp_init, (void *) _init_server_python},
    {Py_tp_dealloc, (void *) _dealloc_server_python},
    {Py_tp_methods, (void *) server_methods},
    {Py_tp_getset, (void *) server_getset},
    {Py_tp_doc, (void *) "WSGI or ASGI server running on the viriatum event loop."},
    {0, NULL}
};

static PyType_Spec server_spec = {
    "viriatum._viriatum.Server",
    sizeof(struct server_python_t),
    0,
    Py_TPFLAGS_DEFAULT,
    server_slots
};

PyObject *create_server_type_python(void) {
    /* creates the server type from the specification defined above,
    the resulting type is a heap allocated one */
    return PyType_FromSpec(&server_spec);
}
