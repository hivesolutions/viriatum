/*
 Hive Viriatum Commons
 Copyright (c) 2008-2020 Hive Solutions Lda.

 This file is part of Hive Viriatum Commons.

 Hive Viriatum Commons is free software: you can redistribute it and/or modify
 it under the terms of the Apache License as published by the Apache
 Foundation, either version 2.0 of the License, or (at your option) any
 later version.

 Hive Viriatum Commons is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 Apache License for more details.

 You should have received a copy of the Apache License along with
 Hive Viriatum Commons. If not, see <http://www.apache.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2020 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#ifdef VIRIATUM_PLATFORM_WIN32
#define LIBRARY_REFERENCE HMODULE
#define LIBRARY_SYMBOL FARPROC
#define LOAD_LIBRARY(library_path) LoadLibrary(library_path)
#define UNLOAD_LIBRARY(library_reference) FreeLibrary(library_reference)
#define GET_LIBRARY_SYMBOL(library_reference, symbol_name) GetProcAddress(library_reference, symbol_name)
#define GET_LIBRARY_ERROR_MESSAGE() NULL
#endif

#ifdef VIRIATUM_PLATFORM_UNIX
#define LIBRARY_REFERENCE void *
#define LIBRARY_SYMBOL void *
#define LOAD_LIBRARY(library_path) dlopen(library_path, RTLD_NOW | RTLD_GLOBAL)
#define UNLOAD_LIBRARY(library_reference) dlclose(library_reference)
#define GET_LIBRARY_SYMBOL(library_reference, symbol_name) dlsym(library_reference, symbol_name)
#define GET_LIBRARY_ERROR_MESSAGE() dlerror()
#endif
