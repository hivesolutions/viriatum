/*
 Hive Viriatum Modules
 Copyright (C) 2008-2015 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2015 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#pragma once

#include "entry.h"

VIRIATUM_EXTERNAL_PREFIX PyMethodDef wsgi_methods[3];
VIRIATUM_EXTERNAL_PREFIX PyMethodDef input_methods[5];

VIRIATUM_EXTERNAL_PREFIX PyTypeObject input_type;

typedef struct wsgi_input_t {
    PyObject_HEAD
    unsigned char *post_data;
    size_t position;
    size_t size;
} wsgi_intput;

PyObject *wsgi_start_response(PyObject *self, PyObject *args);
PyObject *wsgi_write(PyObject *self, PyObject *args);
PyObject *wsgi_file(PyObject *self, PyObject *args);

PyObject *_new_wsgi_input(unsigned char *post_data, size_t size);

PyObject *new_wsgi_input(PyObject *args);
void dealloc_wsgi_input(PyObject *self);

PyObject *wsgi_input_iter(PyObject *self, PyObject *args);
PyObject *wsgi_input_iternext(PyObject *self, PyObject *args);
PyObject *wsgi_input_close(PyObject *self, PyObject *args);
PyObject *wsgi_input_read(PyObject *self, PyObject *args);
PyObject *wsgi_input_readline(PyObject *self, PyObject *args);
PyObject *wsgi_input_readlines(PyObject *self, PyObject *args);
