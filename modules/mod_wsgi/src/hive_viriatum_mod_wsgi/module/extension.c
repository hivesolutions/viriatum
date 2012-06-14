/*
 Hive Viriatum Modules
 Copyright (C) 2008 Hive Solutions Lda.

 This file is part of Hive Viriatum Modules.

 Hive Viriatum Modules is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Hive Viriatum Modules is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Hive Viriatum Modules. If not, see <http://www.gnu.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

#include "extension.h"

PyMethodDef wsgi_methods[3] = {
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
        NULL,
        NULL,
        0,
        NULL
    }
};

PyObject *wsgi_start_response(PyObject *self, PyObject *args) {
    PyObject *wsgi_module;
    PyObject *write_function;

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

    PyObject *header_name;
    PyObject *header_value;
    char *_header_name;
    char *_header_value;

    size_t index;

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



    iterator = PyObject_GetIter(headers);
    if(iterator == NULL) { RAISE_NO_ERROR; }

    index = 0;

    while(1) {
        item = PyIter_Next(iterator);
        if(item == NULL) { break; }
        header_name = PySequence_GetItem(item, 0);
        header_value = PySequence_GetItem(item, 1);

        _header_name = PyString_AsString(header_name);
        _header_value = PyString_AsString(header_value);

        SPRINTF(_wsgi_request.headers[_wsgi_request.header_count], 1024, "%s: %s", _header_name, _header_value);
        _wsgi_request.header_count++;

        Py_XDECREF(header_value);
        Py_XDECREF(header_name);
        Py_DECREF(item);
    }

    Py_DECREF(iterator);




    wsgi_module = PyImport_ImportModule("viriatum_wsgi");
    if(wsgi_module == NULL) { return NULL; }

    write_function = PyObject_GetAttrString(wsgi_module, "write");
    if(!write_function || !PyCallable_Check(write_function)) { RAISE_NO_ERROR; }

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
