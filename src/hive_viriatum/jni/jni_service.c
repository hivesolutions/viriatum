/*
 Hive Viriatum Web Server
 Copyright (C) 2008-2012 Hive Solutions Lda.

 This file is part of Hive Viriatum Web Server.

 Hive Viriatum Web Server is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Hive Viriatum Web Server is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Hive Viriatum Web Server. If not, see <http://www.gnu.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2012 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

#include "jni_service.h"

#ifdef VIRIATUM_JNI

jstring Java_pt_hive_viriatum_http_Service_run(JNIEnv *env, jclass cls) {
    /* allocates the return value */
    ERROR_CODE return_value;

    /* allocates space for the hash map that will hold
    the arguments for the service execution */
    struct hash_map_t *arguments;

    /* allocates space for the result string buffer,
	going to be used to return a possible error message */
    char buffer[1024] = "";

    /* runs the service, with the given arguments, they are
    crated on the fly as an empty map and deleted after*/
    create_hash_map(&arguments, 0);
    return_value = run_service("jni", arguments);
    delete_hash_map(arguments);

    /* tests the error code for error */
    if(IS_ERROR_CODE(return_value)) {
        /* prints an error message and copies it to the return
        value string buffer */
        V_ERROR_F("Problem running service (%s)\n", (char *) GET_ERROR());
        SPRINTF(buffer, 1024, "Problem running service (%s)\n", (char *) GET_ERROR());
    }

    return (*env)->NewStringUTF(env, buffer);
}

jstring Java_pt_hive_viriatum_http_Service_ran(JNIEnv *env, jclass cls) {
    /* allocates the return value */
    ERROR_CODE return_value;

    /* allocates space for the result string buffer,
	going to be used to return a possible error message */
    char buffer[1024] = "";

	/* runs the ran service call in order to start the
	unloading of the service */
    return_value = ran_service();

    /* tests the error code for error */
    if(IS_ERROR_CODE(return_value)) {
        /* prints an error message and copies it to the return
        value string buffer */
        V_ERROR_F("Problem ranning service (%s)\n", (char *) GET_ERROR());
        SPRINTF(buffer, 1024, "Problem ranning service (%s)\n", (char *) GET_ERROR());
    }

    return (*env)->NewStringUTF(env, buffer);
}

jint Java_pt_hive_viriatum_http_Service_port(JNIEnv *env, jclass cls) {
    /* allocates the return value */
    ERROR_CODE return_value;

	/* allocates space for the global service instance
	to be retrieved */
    struct service_t *service;

    /* allocates space for the result string buffer,
	going to be used to return a possible error message */
    char buffer[1024] = "";

	/* tries to retrieve the reference to the global service
	instance, in order to get the port from it */
    return_value = pointer_service(&service);
    if(IS_ERROR_CODE(return_value)) {
        /* prints an error message and copies it to the return
        value string buffer to be returned to java vm */
        V_ERROR_F("Problem getting service (%s)\n", (char *) GET_ERROR());
        SPRINTF(buffer, 1024, "Problem getting service (%s)\n", (char *) GET_ERROR());
    }

	/* returns the currently set port for the service instance
	to the caller method (java) */
    return service->port;
}

#endif
