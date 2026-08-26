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

static int _init_server_python(PyObject *self, PyObject *args, PyObject *kwargs) {
    /* allocates space for the various arguments that may be provided
    for the construction of the server object */
    PyObject *application;
    const char *host = VIRIATUM_DEFAULT_HOST;
    const char *www_root = NULL;
    int port = VIRIATUM_DEFAULT_PORT;

    /* allocates space for the arguments map that is required by the
    default options loading operation */
    struct hash_map_t *arguments;
    struct service_t *service;

    /* defines the complete set of keywords accepted by the constructor
    of the server object (application is the only mandatory one) */
    static char *keywords[] = {"application", "host", "port", "www_root", NULL};

    /* retrieves the reference to the server object that is currently
    being initialized (target of the operation) */
    struct server_python_t *server_python = (struct server_python_t *) self;

    /* parses the arguments provided to the constructor according to
    the keywords sequence defined above */
    if(!PyArg_ParseTupleAndKeywords(
        args, kwargs, "O|sis", keywords, &application, &host, &port, &www_root
    )) { return -1; }

    /* verifies that the provided application is a callable object as
    required by the WSGI specification */
    if(!PyCallable_Check(application)) {
        PyErr_SetString(PyExc_TypeError, "application must be callable");
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
    service->options->address = (unsigned char *) VIRIATUM_DEFAULT_HOST;
    service->options->handler_name = VIRIATUM_PYTHON_HANDLER_NAME;
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

    /* registers the python handler in the service, this operation
    takes a reference on the provided application */
    register_handler_python(service, application);

    /* sets the service in the server object and marks it as not
    opened, the opening happens on the serving operation */
    server_python->service = service;
    server_python->opened = FALSE;

    /* returns the success value as the initialization of the
    server object went without problems */
    return 0;
}

static void _dealloc_server_python(PyObject *self) {
    /* retrieves the reference to the server object and in case there
    is a service set unregisters the handler and releases it */
    struct server_python_t *server_python = (struct server_python_t *) self;
    if(server_python->service != NULL) {
        unregister_handler_python(server_python->service);
        delete_service(server_python->service);
        server_python->service = NULL;
    }

    /* releases the object itself using the type of it */
    Py_TYPE(self)->tp_free(self);
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
    }

    /* iterates continuously while the service is open, the polling
    operation is the only one that may block and so it's the only one
    that runs with the global interpreter lock released */
    while(service->status == STATUS_OPEN) {
        Py_BEGIN_ALLOW_THREADS
        poll_service(service);
        Py_END_ALLOW_THREADS
        call_service(service);

        /* verifies if a signal has been raised in the meantime, this
        is what makes the keyboard interrupt work as expected */
        if(PyErr_CheckSignals() != 0) {
            stop_service(service);
            close_service(service);
            server_python->opened = FALSE;
            return NULL;
        }
    }

    /* closes the service releasing every structure that has been
    created during the opening of it */
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
    {"connections", _connections_server_python, NULL, "The number of currently open connections.", NULL},
    {"uptime", _uptime_server_python, NULL, "The uptime of the server as a string.", NULL},
    {NULL, NULL, NULL, NULL, NULL}
};

static PyType_Slot server_slots[] = {
    {Py_tp_init, (void *) _init_server_python},
    {Py_tp_dealloc, (void *) _dealloc_server_python},
    {Py_tp_methods, (void *) server_methods},
    {Py_tp_getset, (void *) server_getset},
    {Py_tp_doc, (void *) "WSGI server running on the viriatum event loop."},
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
