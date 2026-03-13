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

PyMethodDef wsgi_methods[8] = {
    {
        "start_response",
        wsgi_start_response,
        METH_VARARGS,
        NULL
    },
    {
        "write",
        wsgi_write,
        METH_VARARGS,
        NULL
    },
    {
        "connections",
        wsgi_connections,
        METH_NOARGS,
        NULL
    },
    {
        "connections_l",
        wsgi_connections_l,
        METH_NOARGS,
        NULL
    },
    {
        "connection_info",
        wsgi_connection_info,
        METH_VARARGS,
        NULL
    },
    {
        "uptime",
        wsgi_uptime,
        METH_NOARGS,
        NULL
    },
    {
        "modules",
        wsgi_modules,
        METH_NOARGS,
        NULL
    },
    {
        NULL,
        NULL,
        0,
        NULL
    }
};

PyObject *wsgi_start_response(PyObject *self, PyObject *args) {
    /* allocates space for the WSGI (utils) module and from the write
    function reference contained in it */
    PyObject *wsgi_module;
    PyObject *write_function;

    /* allocates space for the return value as a python reference to
    be returned to the calling python function */
    PyObject *return_value;

    /* allocates space for all the arguments, the status line (error
    code), for the headers list and for the execution information */
    const char *error_code;
    PyObject *headers;
    PyObject *exc_info;

    /* allocates space for the iterator and for the item to be used
    to percolate the list containing the various header tuples */
    PyObject *iterator;
    PyObject *item;

    /* allocates space for both the python representation of the header
    name and value but also for their internal buffers */
    PyObject *header_name;
    PyObject *header_value;
    const char *_header_name;
    const char *_header_value;

    /* allocates space for the result value from the parsing of the
    arguments, in the initial part of the function */
    int result;

    /* start the exc info object to the initial none value (unset)
    this is used because the argument is optional */
    exc_info = Py_None;

    /* parses the arguments, unpacking them into the error code
    and the headers to be used in the function */
    result = PyArg_ParseTuple(args, "sO|O", &error_code, &headers, &exc_info);
    if(result == 0) { return NULL; }

#ifdef VIRIATUM_PLATFORM_MSC
    SSCANF(error_code, "%d %s", &_wsgi_request.status_code, _wsgi_request.status_message, 256);
#else
    SSCANF(error_code, "%d %s", &_wsgi_request.status_code, _wsgi_request.status_message);
#endif

    /* creates the iterator to be used to percolate around
    the various header tuples */
    iterator = PyObject_GetIter(headers);
    if(iterator == NULL) { RAISE_NO_ERROR; }

    /* iterates continuously to percolate around the various
    header tuples */
    while(1) {
        /* retrieves the next iterator item and breaks the
        loop in case an invalid (end of iteration) item is
        found, otherwise retrieves the first and second items
        from the presumably sequence item */
        item = PyIter_Next(iterator);
        if(item == NULL) { break; }
        header_name = PySequence_GetItem(item, 0);
        header_value = PySequence_GetItem(item, 1);

        /* converts the header name and value into a linear
        string buffer so that is possible to format them */
        _header_name = PyUnicode_AsUTF8(header_name);
        _header_value = PyUnicode_AsUTF8(header_value);
        if(_header_name == NULL || _header_value == NULL) {
            Py_XDECREF(header_value);
            Py_XDECREF(header_name);
            Py_DECREF(item);
            continue;
        }

        /* checks if the current header is the content length
        header in such case the length set flag must be set in
        the request so that operation may be taken accordingly */
        _wsgi_request.has_length = strcmp(_header_name, CONTENT_LENGTH_H) == 0 ?
            TRUE : _wsgi_request.has_length;

        /* formats the header into the "normal" format and sets
        it under the headers buffer in the WSGI request */
        SPRINTF(
            _wsgi_request.headers[_wsgi_request.header_count],
            VIRIATUM_MAX_HEADER_C_SIZE,
            "%s: %s",
            _header_name,
            _header_value
        );
        _wsgi_request.header_count++;

        /* updates the reference count of the various elements
        used in the iteration cycle */
        Py_XDECREF(header_value);
        Py_XDECREF(header_name);
        Py_DECREF(item);
    }

    /* releases the iterator by decrementing its reference
    count (avoids memory leak) */
    Py_DECREF(iterator);

    /* imports the WSGI module containing the util methods to be used by the
    application to access viriatum WSGI functions */
    wsgi_module = PyImport_ImportModule("viriatum_wsgi");
    if(wsgi_module == NULL) { return NULL; }

    /* retrieves the reference to the write function from the WSGI module
    and then verifies that it's a valid python function */
    write_function = PyObject_GetAttrString(wsgi_module, "write");
    if(!write_function || !PyCallable_Check(write_function)) {
        Py_XDECREF(write_function);
        Py_DECREF(wsgi_module);
        return NULL;
    }

    /* builds the return value with the write function so
    that the caller function may write directly to the stream */
    return_value = Py_BuildValue("O", write_function);

    /* decrements the reference count of the write function and
    of the module they are not important anymore */
    Py_DECREF(write_function);
    Py_DECREF(wsgi_module);

    /* returns the return value to the caller function, this value
    should include the write function */
    return return_value;
}

PyObject *wsgi_write(PyObject *self, PyObject *args) {
    return Py_BuildValue("i", 0);
}

PyObject *wsgi_connections(PyObject *self, PyObject *args) {
    return Py_BuildValue("l", (long) _service->connections_list->size);
}

PyObject *wsgi_connections_l(PyObject *self, PyObject *args) {
    /* allocates space for the local variables that are
    going to be used in the construction of the connections */
    struct iterator_t *iterator;
    struct connection_t *connection;
    unsigned long long delta;
    char uptime[128];
    char is_empty;

    /* creates a new list to hold the connection dictionaries
    that will be returned to the python caller */
    PyObject *list = PyList_New(0);

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

        /* creates a dictionary with the connection attributes and
        appends it to the result list */
        PyObject *dict = PyDict_New();
        PyObject *_value;

        _value = PyUnicode_FromString(is_empty ? "N/A" : (char *) connection->host);
        PyDict_SetItemString(dict, "host", _value);
        Py_DECREF(_value);

        _value = PyLong_FromLong((long) connection->id);
        PyDict_SetItemString(dict, "id", _value);
        Py_DECREF(_value);

        _value = PyUnicode_FromString(uptime);
        PyDict_SetItemString(dict, "uptime", _value);
        Py_DECREF(_value);

        PyList_Append(list, dict);
        Py_DECREF(dict);
    }

    /* deletes the iterator for the connections list in order to
    avoid any memory leak that could arise from this */
    delete_iterator_linked_list(_service->connections_list, iterator);

    return list;
}

PyObject *wsgi_connection_info(PyObject *self, PyObject *args) {
    /* allocates space for the local variables that are
    going to be used in the construction of the connection info */
    long id;
    char is_empty;
    struct iterator_t *iterator;
    struct connection_t *connection;
    unsigned long long delta;
    char uptime[128];

    /* parses the id argument from the python call */
    if(!PyArg_ParseTuple(args, "l", &id)) { return NULL; }

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

    /* in case no connection was found returns none to indicate
    that the requested connection does not exist */
    if(connection == NULL || (long) connection->id != id) { Py_RETURN_NONE; }

    /* retrieves the delta value by calculating the difference between
    the current time and the creation time then uses it to calculate
    the uptime for the connection as a string description */
    delta = (unsigned long long) time(NULL) - connection->creation;
    format_delta(uptime, sizeof(uptime), delta, 2);

    /* verifies if the current host is empty, this is a special
    case where no resolution of the value was possible */
    is_empty = connection->host[0] == '\0';

    /* creates a dictionary with the connection attributes and
    returns it to the python caller */
    PyObject *dict = PyDict_New();
    PyObject *_value;

    _value = PyUnicode_FromString(is_empty ? "N/A" : (char *) connection->host);
    PyDict_SetItemString(dict, "host", _value);
    Py_DECREF(_value);

    _value = PyLong_FromLong((long) connection->id);
    PyDict_SetItemString(dict, "id", _value);
    Py_DECREF(_value);

    _value = PyUnicode_FromString(uptime);
    PyDict_SetItemString(dict, "uptime", _value);
    Py_DECREF(_value);

    return dict;
}

PyObject *wsgi_uptime(PyObject *self, PyObject *args) {
    return PyUnicode_FromString(_service->get_uptime(_service, 2));
}

PyObject *wsgi_modules(PyObject *self, PyObject *args) {
    return PyUnicode_FromString((char *) _service->modules);
}

PyObject *_new_wsgi_input(unsigned char *post_data, size_t size) {
    struct wsgi_input_t *self;

    self = PyObject_New(struct wsgi_input_t, &input_type);
    self->post_data = post_data;
    self->size = size;
    self->position = 0;

    return (PyObject *) self;
}

PyObject *new_wsgi_input(PyObject *args) {
    return (PyObject *) _new_wsgi_input(NULL, 0);
}

void dealloc_wsgi_input(PyObject *self) {
    PyObject_Del(self);
}

PyObject *wsgi_input_iter(PyObject *self, PyObject *args) {
    Py_RETURN_NONE;
}

PyObject *wsgi_input_iternext(PyObject *self, PyObject *args) {
    Py_RETURN_NONE;
}

PyObject *wsgi_input_close(PyObject *self, PyObject *args) {
    Py_RETURN_NONE;
}

PyObject *wsgi_input_read(PyObject *self, PyObject *args) {
    struct wsgi_input_t *_self = (struct wsgi_input_t *) self;

    size_t remaining = _self->size - _self->position;
    PyObject *buffer = PyBytes_FromStringAndSize((char *) &_self->post_data[_self->position], remaining);
    _self->position += remaining;

    return buffer;
}

PyObject *wsgi_input_readline(PyObject *self, PyObject *args) {
    Py_RETURN_NONE;
}

PyObject *wsgi_input_readlines(PyObject *self, PyObject *args) {
    Py_RETURN_NONE;
}

PyMethodDef input_methods[5] = {
    {
        "close",
        (PyCFunction) wsgi_input_close,
        METH_VARARGS,
        0
    },
    {
        "read",
        (PyCFunction) wsgi_input_read,
        METH_VARARGS,
        0
    },
    {
        "readline",
        (PyCFunction) wsgi_input_readline,
        METH_VARARGS,
        0
    },
    {
        "readlines",
        (PyCFunction) wsgi_input_readlines,
        METH_VARARGS,
        0
    },
    {
        NULL,
        NULL,
        0,
        NULL
    }
};

PyTypeObject input_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "viriatum_wsgi.input",                     /* tp_name */
    sizeof(struct wsgi_input_t),               /* tp_basicsize */
    0,                                         /* tp_itemsize*/
    (destructor) dealloc_wsgi_input,           /* tp_dealloc */
    0,                                         /* tp_vectorcall_offset */
    0,                                         /* tp_getattr */
    0,                                         /* tp_setattr */
    0,                                         /* tp_as_async */
    0,                                         /* tp_repr */
    0,                                         /* tp_as_number */
    0,                                         /* tp_as_sequence */
    0,                                         /* tp_as_mapping */
    0,                                         /* tp_hash */
    0,                                         /* tp_call */
    0,                                         /* tp_str */
    0,                                         /* tp_getattro */
    0,                                         /* tp_setattro */
    0,                                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                        /* tp_flags */
    0,                                         /* tp_doc */
    0,                                         /* tp_traverse */
    0,                                         /* tp_clear */
    0,                                         /* tp_richcompare */
    0,                                         /* tp_weaklistoffset */
    (getiterfunc) wsgi_input_iter,             /* tp_iter */
    (iternextfunc) wsgi_input_iternext,        /* tp_iternext */
    input_methods,                             /* tp_methods */
    0,                                         /* tp_members */
    0,                                         /* tp_getset */
    0,                                         /* tp_base */
    0,                                         /* tp_dict */
    0,                                         /* tp_descr_get */
    0,                                         /* tp_descr_set */
    0,                                         /* tp_dictoffset */
    0,                                         /* tp_init */
    0,                                         /* tp_alloc */
    (newfunc) new_wsgi_input,                  /* tp_new */
    0,                                         /* tp_free */
    0,                                         /* tp_is_gc */
};
