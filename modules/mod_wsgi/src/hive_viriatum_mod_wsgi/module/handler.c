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

#include "handler.h"

ERROR_CODE create_mod_wsgi_http_handler(struct mod_wsgi_http_handler_t **mod_wsgi_http_handler_pointer, struct http_handler_t *http_handler) {
    /* retrieves the mod wsgi http handler size */
    size_t mod_wsgi_http_handler_size = sizeof(struct mod_wsgi_http_handler_t);

    /* allocates space for the mod wsgi http handler */
    struct mod_wsgi_http_handler_t *mod_wsgi_http_handler = (struct mod_wsgi_http_handler_t *) MALLOC(mod_wsgi_http_handler_size);

    /* sets the mod wsgi http handler in the upper http handler substrate */
    http_handler->lower = (void *) mod_wsgi_http_handler;

    /* sets the mod wsgi http handler in the mod wsgi http handler pointer */
    *mod_wsgi_http_handler_pointer = mod_wsgi_http_handler;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_mod_wsgi_http_handler(struct mod_wsgi_http_handler_t *mod_wsgi_http_handler) {
    /* releases the mod wsgi http handler */
    FREE(mod_wsgi_http_handler);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE create_handler_wsgi_context(struct handler_wsgi_context_t **handler_wsgi_context_pointer) {
    /* retrieves the handler wsgi context size */
    size_t handler_wsgi_context_size = sizeof(struct handler_wsgi_context_t);

    /* allocates space for the handler wsgi context */
    struct handler_wsgi_context_t *handler_wsgi_context = (struct handler_wsgi_context_t *) MALLOC(handler_wsgi_context_size);

    /* sets the handler wsgi context in the  pointer */
    *handler_wsgi_context_pointer = handler_wsgi_context;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE delete_handler_wsgi_context(struct handler_wsgi_context_t *handler_wsgi_context) {
    /* releases the handler wsgi context memory */
    FREE(handler_wsgi_context);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE set_handler_module(struct http_connection_t *http_connection) {
    /* sets the http parser values */
    _set_http_parser_handler_module(http_connection->http_parser);

    /* sets the http settings values */
    _set_http_settings_handler_module(http_connection->http_settings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE unset_handler_module(struct http_connection_t *http_connection) {
    /* unsets the http parser values */
    _unset_http_parser_handler_module(http_connection->http_parser);

    /* unsets the http settings values */
    _unset_http_settings_handler_module(http_connection->http_settings);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE message_begin_callback_handler_module(struct http_parser_t *http_parser) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE url_callback_handler_module(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE header_field_callback_handler_module(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE header_value_callback_handler_module(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE headers_complete_callback_handler_module(struct http_parser_t *http_parser) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE body_callback_handler_module(struct http_parser_t *http_parser, const unsigned char *data, size_t data_size) {
    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE message_complete_callback_handler_module(struct http_parser_t *http_parser) {
    /* sends (and creates) the reponse */
    _send_response_handler_module(http_parser);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _set_http_parser_handler_module(struct http_parser_t *http_parser) {
    /* allocates space for the handler wsgi context and
    then creates and populates the instance after that
    sets the handler file context as the context for
    the http parser*/
    struct handler_wsgi_context_t *handler_wsgi_context;
    create_handler_wsgi_context(&handler_wsgi_context);
    http_parser->context = handler_wsgi_context;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unset_http_parser_handler_module(struct http_parser_t *http_parser) {
    /* retrieves the handler wsgi context from the http parser
    and then deletes (releases memory) */
    struct handler_wsgi_context_t *handler_wsgi_context = (struct handler_wsgi_context_t *) http_parser->context;
    delete_handler_wsgi_context(handler_wsgi_context);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _set_http_settings_handler_module(struct http_settings_t *http_settings) {
    /* sets the various callback functions in the http settings
    structure, these callbacks are going to be used in the runtime
    processing of http parser (runtime execution) */
    http_settings->on_message_begin = message_begin_callback_handler_module;
    http_settings->on_url = url_callback_handler_module;
    http_settings->on_header_field = header_field_callback_handler_module;
    http_settings->on_header_value = header_value_callback_handler_module;
    http_settings->on_headers_complete = headers_complete_callback_handler_module;
    http_settings->on_body = body_callback_handler_module;
    http_settings->on_message_complete = message_complete_callback_handler_module;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _unset_http_settings_handler_module(struct http_settings_t *http_settings) {
    /* unsets the various callback functions from the http settings */
    http_settings->on_message_begin = NULL;
    http_settings->on_url = NULL;
    http_settings->on_header_field = NULL;
    http_settings->on_header_value = NULL;
    http_settings->on_headers_complete = NULL;
    http_settings->on_body = NULL;
    http_settings->on_message_complete = NULL;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _send_data_callback(struct connection_t *connection, struct data_t *data, void *parameters) {
    /* raises no error */
    RAISE_NO_ERROR;
}



static PyObject *wsgi_start_response(PyObject *self, PyObject *args) {
	PyObject *wsgi_module;
	PyObject *write_function;
	
	PyObject *return_value;

	/* allocates space for both arguments for the start response
	function, the status line (error code) and for the headers list */
    const char *error_code;
    PyObject *headers;

	/* allocates space for the result value from the parsing of the
	arguments, in the initial part of the function */
	int result;

	result = PyArg_ParseTuple(args, "sO", &error_code, &headers);
	if(result == 0) { return NULL; }

	printf("---%s---\n", error_code);

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

static PyObject *wsgi_write(PyObject *self, PyObject *args) {
    return Py_BuildValue("i", 23);
}

static PyMethodDef wsgi_methods[] = {
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

ERROR_CODE _send_response_handler_module(struct http_parser_t *http_parser) {
	PyObject *module;
	PyObject *wsgi_module;
	PyObject *handler_function;
	PyObject *start_response_function;

	PyObject *args;
	PyObject *environ;
	PyObject *result;

	/* TODO: ISTO TEM DE SER METIDO NO INCIO do interpretador
	E NO FICHEIRO extension.c */
	Py_InitModule("viriatum_wsgi", wsgi_methods);

	wsgi_module = PyImport_ImportModule("viriatum_wsgi");
	if(wsgi_module == NULL) { RAISE_NO_ERROR; }

    start_response_function = PyObject_GetAttrString(wsgi_module, "start_response");
	if(!start_response_function || !PyCallable_Check(start_response_function)) { RAISE_NO_ERROR; }

	/* imports the associated (handler) module and retrieves
	its reference to be used for the calling, in case the
	reference is invalid raises an error */
	module = PyImport_ImportModule("tobias");
	if(module == NULL) { RAISE_NO_ERROR; }

	/* retrieves the function to be used as handler for the
	wsgi request, then check if the reference is valid and
	if it refers a valid function */
    handler_function = PyObject_GetAttrString(module, "simple_app");
	if(!handler_function || !PyCallable_Check(handler_function)) { RAISE_NO_ERROR; }

	/* creates a new tuple to hold the various arguments to
	the call and then populates it with the environment variables
	dictionary and with the start response function */
	args = PyTuple_New(2);
	environ = PyDict_New();
	PyTuple_SetItem(args, 0, environ);
	PyTuple_SetItem(args, 1, start_response_function);

	/* calls the handler function retrieving the result and releasing
	the resources immediately in case the result is not valid */
	result = PyObject_CallObject(handler_function, args);
	if(result == NULL) {
		Py_DECREF(handler_function);
		Py_DECREF(args);
		Py_DECREF(module);
		Py_DECREF(wsgi_module);
		RAISE_NO_ERROR;
	}

	/* releases the references on the various resources used in this
	function (memory will be deallocated if necessary) */
	Py_DECREF(handler_function);
	Py_DECREF(args);
	Py_DECREF(module);
	Py_DECREF(wsgi_module);

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _send_response_callback_handler_module(struct connection_t *connection, struct data_t *data, void *parameters) {
    /* raise no error */
    RAISE_NO_ERROR;
}
