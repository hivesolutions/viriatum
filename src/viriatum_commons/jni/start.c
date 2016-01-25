/*
 Hive Viriatum Commons
 Copyright (C) 2008-2016 Hive Solutions Lda.

 This file is part of Hive Viriatum Commons.

 Hive Viriatum Commons is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Hive Viriatum Commons is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Hive Viriatum Commons. If not, see <http://www.gnu.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __version__   = 1.0.0
 __revision__  = $LastChangedRevision$
 __date__      = $LastChangedDate$
 __copyright__ = Copyright (c) 2008-2016 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#include "stdafx.h"

#include "info.h"

#ifdef VIRIATUM_JNI

static JavaVM *java_vm;

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    /* allocates space for the current java environment */
    JNIEnv *env;

    /* saves the current java vm reference */
    java_vm = vm;

    /* retrieves the environment value and checks
    the jni version in case it fails return in error */
    if((*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        /* returns in error */
        return -1;
    }

    /* returns the jni version */
    return JNI_VERSION_1_4;
}

#endif
