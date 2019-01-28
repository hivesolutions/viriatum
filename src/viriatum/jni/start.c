/*
 Hive Viriatum Web Server
 Copyright (c) 2008-2019 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2019 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "start.h"

#ifdef VIRIATUM_JNI

static JavaVM *java_vm;

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    /* allocates space for the current java environment */
    JNIEnv *env;

    /* saves the current java vm reference */
    java_vm = vm;

    /* retrieves the environment value and checks
    the jni version in case it fails return in error */
    if((*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_4) != JNI_OK){
        /* returns in error */
        return -1;
    }

    /* returns the jni version */
    return JNI_VERSION_1_4;
}

#endif
