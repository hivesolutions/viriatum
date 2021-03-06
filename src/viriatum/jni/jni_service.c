/*
 Hive Viriatum Web Server
 Copyright (c) 2008-2020 Hive Solutions Lda.

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
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2020 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "jni_service.h"

#ifdef VIRIATUM_JNI

jboolean Java_pt_hive_viriatum_http_Service_exists(JNIEnv *env, jclass cls) {
    ERROR_CODE return_value;
    struct service_t *service;
    return_value = pointer_service(&service);
    if(IS_ERROR_CODE(return_value)) { return FALSE; }
    if(service == NULL) { return FALSE; }
    else { return TRUE; }
}

jstring Java_pt_hive_viriatum_http_Service_init(JNIEnv *env, jclass cls) {
    /* allocates the return value */
    ERROR_CODE return_value;

    /* allocates space for the hash map that will hold
    the arguments for the service execution */
    struct hash_map_t *arguments;

    /* allocates space for the result string buffer,
    going to be used to return a possible error message */
    char buffer[1024] = "";

    /* runs the service, with the given arguments, they are
    crated on the fly as an empty map and deleted after */
    create_hash_map(&arguments, 0);
    return_value = init_service("jni", arguments);
    delete_hash_map(arguments);

    /* tests the error code for error */
    if(IS_ERROR_CODE(return_value)) {
        /* prints an error message and copies it to the return
        value string buffer */
        V_ERROR_F(
            "Problem initializing service (%s)\n",
            (char *) GET_ERROR()
        );
        SPRINTF(
            buffer,
            1024,
            "Problem initializing service (%s)\n",
            (char *) GET_ERROR()
        );
    }

    return (*env)->NewStringUTF(env, buffer);
}

jstring Java_pt_hive_viriatum_http_Service_destroy(JNIEnv *env, jclass cls) {
    /* allocates the return value */
    ERROR_CODE return_value;

    /* allocates space for the result string buffer,
    going to be used to return a possible error message */
    char buffer[1024] = "";

    /* destroys the service structure, releasing all of
    its structure memory and disabling the run of it */
    return_value = destroy_service();

    /* tests the error code for error */
    if(IS_ERROR_CODE(return_value)) {
        /* prints an error message and copies it to the return
        value string buffer */
        V_ERROR_F(
            "Problem destroying service (%s)\n",
            (char *) GET_ERROR()
        );
        SPRINTF(
            buffer,
            1024,
            "Problem destroying service (%s)\n",
            (char *) GET_ERROR()
        );
    }

    return (*env)->NewStringUTF(env, buffer);
}

jstring Java_pt_hive_viriatum_http_Service_run(JNIEnv *env, jclass cls) {
    /* allocates the return value */
    ERROR_CODE return_value;

    /* allocates space for the result string buffer,
    going to be used to return a possible error message */
    char buffer[1024] = "";

    /* runs the service, from the current global service
    structure, this structure must already be initialized */
    return_value = run_service();

    /* tests the error code for error */
    if(IS_ERROR_CODE(return_value)) {
        /* prints an error message and copies it to the return
        value string buffer */
        V_ERROR_F(
            "Problem running service (%s)\n",
            (char *) GET_ERROR()
        );
        SPRINTF(
            buffer,
            1024,
            "Problem running service (%s)\n",
            (char *) GET_ERROR()
        );
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
        V_ERROR_F(
            "Problem ranning service (%s)\n",
            (char *) GET_ERROR()
        );
        SPRINTF(
            buffer,
            1024,
            "Problem ranning service (%s)\n",
            (char *) GET_ERROR()
        );
    }

    return (*env)->NewStringUTF(env, buffer);
}

jint Java_pt_hive_viriatum_http_Service_connections(JNIEnv *env, jclass cls) {
    /* allocates the return value */
    ERROR_CODE return_value;

    /* allocates space for the global service instance
    to be retrieved */
    struct service_t *service;

    /* tries to retrieve the reference to the global service
    instance, in order to get the port from it */
    return_value = pointer_service(&service);
    if(IS_ERROR_CODE(return_value)) { return -1; }

    /* in case the service structure or the connections
    list in it are not currently set returns an error value */
    if(service == NULL || service->connections_list == NULL) { return -1; }

    /* returns the currently set number connections for the
    service instance to the caller method (java) */
    return service->connections_list->size;
}

jstring Java_pt_hive_viriatum_http_Service_uptime(JNIEnv *env, jclass cls) {
    /* allocates the return value */
    ERROR_CODE return_value;

    /* allocates space for the global service instance
    to be retrieved */
    struct service_t *service;

    /* allocates space for the result uptime buffer,
    this value should be a global constant */
    const char *uptime;

    /* tries to retrieve the reference to the global service
    instance, in order to get the port from it */
    return_value = pointer_service(&service);
    if(IS_ERROR_CODE(return_value)) { return NULL; }

    /* in case the service structure is not currently
    set returns an error value */
    if(service == NULL) { return NULL; }

    /* retrieves the uptime from the service as a two
    value string and then returns it to the caller method */
    uptime = service->get_uptime(service, 2);
    return (*env)->NewStringUTF(env, uptime);
}

jstring Java_pt_hive_viriatum_http_Service_name(JNIEnv *env, jclass cls) {
    /* allocates the return value */
    ERROR_CODE return_value;

    /* allocates space for the global service instance
    to be retrieved */
    struct service_t *service;

    /* tries to retrieve the reference to the global service
    instance, in order to get the port from it */
    return_value = pointer_service(&service);
    if(IS_ERROR_CODE(return_value)) { return NULL; }

    /* in case the service structure is not currently
    set returns an error value */
    if(service == NULL) { return NULL; }

    /* returns the currently set name for the service instance
    to the caller method (java) */
    return (*env)->NewStringUTF(env, (char *) service->name);
}

jstring Java_pt_hive_viriatum_http_Service_version(JNIEnv *env, jclass cls) {
    /* allocates the return value */
    ERROR_CODE return_value;

    /* allocates space for the global service instance
    to be retrieved */
    struct service_t *service;

    /* tries to retrieve the reference to the global service
    instance, in order to get the port from it */
    return_value = pointer_service(&service);
    if(IS_ERROR_CODE(return_value)) { return NULL; }

    /* in case the service structure is not currently
    set returns an error value */
    if(service == NULL) { return NULL; }

    /* returns the currently set version for the service instance
    to the caller method (java) */
    return (*env)->NewStringUTF(env, (char *) service->version);
}

jstring Java_pt_hive_viriatum_http_Service_platform(JNIEnv *env, jclass cls) {
    /* allocates the return value */
    ERROR_CODE return_value;

    /* allocates space for the global service instance
    to be retrieved */
    struct service_t *service;

    /* tries to retrieve the reference to the global service
    instance, in order to get the port from it */
    return_value = pointer_service(&service);
    if(IS_ERROR_CODE(return_value)) { return NULL; }

    /* in case the service structure is not currently
    set returns an error value */
    if(service == NULL) { return NULL; }

    /* returns the currently set platform for the service instance
    to the caller method (java) */
    return (*env)->NewStringUTF(env, (char *) service->platform);
}

jstring Java_pt_hive_viriatum_http_Service_flags(JNIEnv *env, jclass cls) {
    /* allocates the return value */
    ERROR_CODE return_value;

    /* allocates space for the global service instance
    to be retrieved */
    struct service_t *service;

    /* tries to retrieve the reference to the global service
    instance, in order to get the port from it */
    return_value = pointer_service(&service);
    if(IS_ERROR_CODE(return_value)) { return NULL; }

    /* in case the service structure is not currently
    set returns an error value */
    if(service == NULL) { return NULL; }

    /* returns the currently set flags for the service instance
    to the caller method (java) */
    return (*env)->NewStringUTF(env, (char *) service->flags);
}

jstring Java_pt_hive_viriatum_http_Service_compiler(JNIEnv *env, jclass cls) {
    /* allocates the return value */
    ERROR_CODE return_value;

    /* allocates space for the global service instance
    to be retrieved */
    struct service_t *service;

    /* tries to retrieve the reference to the global service
    instance, in order to get the port from it */
    return_value = pointer_service(&service);
    if(IS_ERROR_CODE(return_value)) { return NULL; }

    /* in case the service structure is not currently
    set returns an error value */
    if(service == NULL) { return NULL; }

    /* returns the currently set compiler for the service instance
    to the caller method (java) */
    return (*env)->NewStringUTF(env, (char *) service->compiler);
}

jstring Java_pt_hive_viriatum_http_Service_compilerVersion(JNIEnv *env, jclass cls) {
    /* allocates the return value */
    ERROR_CODE return_value;

    /* allocates space for the global service instance
    to be retrieved */
    struct service_t *service;

    /* tries to retrieve the reference to the global service
    instance, in order to get the port from it */
    return_value = pointer_service(&service);
    if(IS_ERROR_CODE(return_value)) { return NULL; }

    /* in case the service structure is not currently
    set returns an error value */
    if(service == NULL) { return NULL; }

    /* returns the currently set compiler version for the service instance
    to the caller method (java) */
    return (*env)->NewStringUTF(env, (char *) service->compiler_version);
}

jstring Java_pt_hive_viriatum_http_Service_compilationDate(JNIEnv *env, jclass cls) {
    /* allocates the return value */
    ERROR_CODE return_value;

    /* allocates space for the global service instance
    to be retrieved */
    struct service_t *service;

    /* tries to retrieve the reference to the global service
    instance, in order to get the port from it */
    return_value = pointer_service(&service);
    if(IS_ERROR_CODE(return_value)) { return NULL; }

    /* in case the service structure is not currently
    set returns an error value */
    if(service == NULL) { return NULL; }

    /* returns the currently set compilation date for the service instance
    to the caller method (java) */
    return (*env)->NewStringUTF(env, (char *) service->compilation_date);
}

jstring Java_pt_hive_viriatum_http_Service_compilationTime(JNIEnv *env, jclass cls) {
    /* allocates the return value */
    ERROR_CODE return_value;

    /* allocates space for the global service instance
    to be retrieved */
    struct service_t *service;

    /* tries to retrieve the reference to the global service
    instance, in order to get the port from it */
    return_value = pointer_service(&service);
    if(IS_ERROR_CODE(return_value)) { return NULL; }

    /* in case the service structure is not currently
    set returns an error value */
    if(service == NULL) { return NULL; }

    /* returns the currently set compilation time for the service instance
    to the caller method (java) */
    return (*env)->NewStringUTF(env, (char *) service->compilation_time);
}

jstring Java_pt_hive_viriatum_http_Service_compilationFlags(JNIEnv *env, jclass cls) {
    /* allocates the return value */
    ERROR_CODE return_value;

    /* allocates space for the global service instance
    to be retrieved */
    struct service_t *service;

    /* tries to retrieve the reference to the global service
    instance, in order to get the port from it */
    return_value = pointer_service(&service);
    if(IS_ERROR_CODE(return_value)) { return NULL; }

    /* in case the service structure is not currently
    set returns an error value */
    if(service == NULL) { return NULL; }

    /* returns the currently set compilation flags for the service instance
    to the caller method (java) */
    return (*env)->NewStringUTF(env, (char *) service->compilation_flags);
}

jstring Java_pt_hive_viriatum_http_Service_modules(JNIEnv *env, jclass cls) {
    /* allocates the return value */
    ERROR_CODE return_value;

    /* allocates space for the global service instance
    to be retrieved */
    struct service_t *service;

    /* tries to retrieve the reference to the global service
    instance, in order to get the port from it */
    return_value = pointer_service(&service);
    if(IS_ERROR_CODE(return_value)) { return NULL; }

    /* in case the service structure is not currently
    set returns an error value */
    if(service == NULL) { return NULL; }

    /* returns the currently set modules for the service instance
    to the caller method (java) */
    return (*env)->NewStringUTF(env, (char *) service->modules);
}

jstring Java_pt_hive_viriatum_http_Service_description(JNIEnv *env, jclass cls) {
    /* allocates the return value */
    ERROR_CODE return_value;

    /* allocates space for the global service instance
    to be retrieved */
    struct service_t *service;

    /* tries to retrieve the reference to the global service
    instance, in order to get the port from it */
    return_value = pointer_service(&service);
    if(IS_ERROR_CODE(return_value)) { return NULL; }

    /* in case the service structure is not currently
    set returns an error value */
    if(service == NULL) { return NULL; }

    /* returns the currently set description for the service instance
    to the caller method (java) */
    return (*env)->NewStringUTF(env, (char *) service->description);
}

jint Java_pt_hive_viriatum_http_Service_status(JNIEnv *env, jclass cls) {
    /* allocates the return value */
    ERROR_CODE return_value;

    /* allocates space for the global service instance
    to be retrieved */
    struct service_t *service;

    /* tries to retrieve the reference to the global service
    instance, in order to get the port from it */
    return_value = pointer_service(&service);
    if(IS_ERROR_CODE(return_value)) { return -1; }

    /* in case the service structure is not currently
    set returns an error value */
    if(service == NULL) { return -1; }

    /* returns the currently set status for the service instance
    to the caller method (java) */
    return service->status;
}

jint Java_pt_hive_viriatum_http_Service_port(JNIEnv *env, jclass cls) {
    /* allocates the return value */
    ERROR_CODE return_value;

    /* allocates space for the global service instance
    to be retrieved */
    struct service_t *service;

    /* tries to retrieve the reference to the global service
    instance, in order to get the port from it */
    return_value = pointer_service(&service);
    if(IS_ERROR_CODE(return_value)) { return -1; }

    /* in case the service options or the service is
    not set returns an error value */
    if(service == NULL || service->options == NULL) { return -1; }

    /* returns the currently set port for the service instance
    to the caller method (java) */
    return service->options->port;
}

#endif
