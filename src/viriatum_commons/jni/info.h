/*
 Hive Viriatum Commons
 Copyright (c) 2008-2018 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2018 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#ifdef VIRIATUM_JNI

#include <jni.h>

VIRIATUM_EXPORT_PREFIX jstring Java_pt_hive_viriatum_commons_Info_getName(JNIEnv *env, jclass cls);
VIRIATUM_EXPORT_PREFIX jstring Java_pt_hive_viriatum_commons_Info_getVersion(JNIEnv *env, jclass cls);
VIRIATUM_EXPORT_PREFIX jstring Java_pt_hive_viriatum_commons_Info_getDescription(JNIEnv *env, jclass cls);
VIRIATUM_EXPORT_PREFIX jstring Java_pt_hive_viriatum_commons_Info_hello(JNIEnv *env, jobject thiz);

#endif
