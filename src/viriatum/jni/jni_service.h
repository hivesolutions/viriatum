/*
 Hive Viriatum Web Server
 Copyright (c) 2008-2016 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2016 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#include "../base/base.h"
#include "../system/system.h"

#ifdef VIRIATUM_JNI

#include <jni.h>

VIRIATUM_EXPORT_PREFIX jboolean Java_pt_hive_viriatum_http_Service_exists(JNIEnv *env, jclass cls);
VIRIATUM_EXPORT_PREFIX jstring Java_pt_hive_viriatum_http_Service_init(JNIEnv *env, jclass cls);
VIRIATUM_EXPORT_PREFIX jstring Java_pt_hive_viriatum_http_Service_destroy(JNIEnv *env, jclass cls);
VIRIATUM_EXPORT_PREFIX jstring Java_pt_hive_viriatum_http_Service_run(JNIEnv *env, jclass cls);
VIRIATUM_EXPORT_PREFIX jstring Java_pt_hive_viriatum_http_Service_ran(JNIEnv *env, jclass cls);
VIRIATUM_EXPORT_PREFIX jint Java_pt_hive_viriatum_http_Service_connections(JNIEnv *env, jclass cls);
VIRIATUM_EXPORT_PREFIX jstring Java_pt_hive_viriatum_http_Service_uptime(JNIEnv *env, jclass cls);
VIRIATUM_EXPORT_PREFIX jstring Java_pt_hive_viriatum_http_Service_name(JNIEnv *env, jclass cls);
VIRIATUM_EXPORT_PREFIX jstring Java_pt_hive_viriatum_http_Service_version(JNIEnv *env, jclass cls);
VIRIATUM_EXPORT_PREFIX jstring Java_pt_hive_viriatum_http_Service_platform(JNIEnv *env, jclass cls);
VIRIATUM_EXPORT_PREFIX jstring Java_pt_hive_viriatum_http_Service_flags(JNIEnv *env, jclass cls);
VIRIATUM_EXPORT_PREFIX jstring Java_pt_hive_viriatum_http_Service_compiler(JNIEnv *env, jclass cls);
VIRIATUM_EXPORT_PREFIX jstring Java_pt_hive_viriatum_http_Service_compilerVersion(JNIEnv *env, jclass cls);
VIRIATUM_EXPORT_PREFIX jstring Java_pt_hive_viriatum_http_Service_compilationDate(JNIEnv *env, jclass cls);
VIRIATUM_EXPORT_PREFIX jstring Java_pt_hive_viriatum_http_Service_compilationTime(JNIEnv *env, jclass cls);
VIRIATUM_EXPORT_PREFIX jstring Java_pt_hive_viriatum_http_Service_compilationFlags(JNIEnv *env, jclass cls);
VIRIATUM_EXPORT_PREFIX jstring Java_pt_hive_viriatum_http_Service_modules(JNIEnv *env, jclass cls);
VIRIATUM_EXPORT_PREFIX jstring Java_pt_hive_viriatum_http_Service_description(JNIEnv *env, jclass cls);
VIRIATUM_EXPORT_PREFIX jint Java_pt_hive_viriatum_http_Service_status(JNIEnv *env, jclass cls);
VIRIATUM_EXPORT_PREFIX jint Java_pt_hive_viriatum_http_Service_port(JNIEnv *env, jclass cls);

#endif
