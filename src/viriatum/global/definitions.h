/*
 Hive Viriatum Web Server
 Copyright (c) 2008-2026 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#pragma once

#ifdef VIRIATUM_PLATFORM_WIN32

VIRIATUM_EXTERNAL_PREFIX unsigned char local;

static char base_path[VIRIATUM_MAX_PATH_SIZE] = {'\0'};
static char resources_path[VIRIATUM_MAX_PATH_SIZE] = {'\0'};
static char modules_path[VIRIATUM_MAX_PATH_SIZE] = {'\0'};
static char config_path[VIRIATUM_MAX_PATH_SIZE] = {'\0'};

static __inline char *get_base_path(void) {
    size_t base_path_length;
    size_t index;

    if(base_path[0] != '\0') { return base_path; }

    GetModuleFileName(NULL, base_path, VIRIATUM_MAX_PATH_SIZE);
    base_path_length = strlen(base_path);

    /* walks the path backwards looking for the separator that closes
    the directory the executable sits in, the index counts one above
    the character it is looking at so that it never wraps around */
    for(index = base_path_length; index > 0; index--) {
        if(base_path[index - 1] != '\\') { continue; }
        break;
    }

    base_path[index > 0 ? index - 1 : 0] = '\0';

    return base_path;
}

static __inline char *get_contents_path(void) {
    const char *base_path;

    if(resources_path[0] != '\0') { return resources_path; }

    base_path = local ? "." : get_base_path();
    if(local) {
        SPRINTF(resources_path, VIRIATUM_MAX_PATH_SIZE, "%s", base_path);
    } else {
        SPRINTF(resources_path, VIRIATUM_MAX_PATH_SIZE, "%s" VIRIATUM_PATH_SEPARATOR "htdocs", base_path);
    }

    return resources_path;
}

static __inline char *get_resources_path(void) {
    const char *base_path;

    if(resources_path[0] != '\0') { return resources_path; }

    base_path = get_base_path();
    SPRINTF(resources_path, VIRIATUM_MAX_PATH_SIZE, "%s" VIRIATUM_PATH_SEPARATOR "htdocs", base_path);

    return resources_path;
}

static __inline char *get_modules_path(void) {
    const char *base_path;

    if(modules_path[0] != '\0') { return modules_path; }

    base_path = get_base_path();
    SPRINTF(modules_path, VIRIATUM_MAX_PATH_SIZE, "%s" VIRIATUM_PATH_SEPARATOR "modules", base_path);

    return modules_path;
}

static __inline char *get_config_path(void) {
    const char *base_path;

    if(config_path[0] != '\0') { return config_path; }

    base_path = get_base_path();
    SPRINTF(config_path, VIRIATUM_MAX_PATH_SIZE, "%s" VIRIATUM_PATH_SEPARATOR "config", base_path);

    return config_path;
}

#ifndef VIRIATUM_MODULES_PATH
#define VIRIATUM_MODULES_PATH get_modules_path()
#endif
#ifndef VIRIATUM_RESOURCES_PATH
#define VIRIATUM_RESOURCES_PATH get_resources_path()
#endif
#ifndef VIRIATUM_CONFIG_PATH
#define VIRIATUM_CONFIG_PATH get_config_path()
#endif
#define VIRIATUM_BASE_PATH ""
#define VIRIATUM_LISTING_PATH "/templates/listing.html.tpl"
#define VIRIATUM_ERROR_PATH "/templates/error.html.tpl"
#ifndef VIRIATUM_CONTENTS_PATH
#define VIRIATUM_CONTENTS_PATH get_contents_path()
#endif
#define VIRIATUM_PID_PATH "viriatum.pid"
#define VIRIATUM_MODULE_PREFIX sizeof("viriatum_mod_") - 1
#endif

#ifdef VIRIATUM_PLATFORM_UNIX
#ifdef VIRIATUM_PLATFORM_ANDROID
#define VIRIATUM_DATA_PATH "/data/data/pt.hive.viriatum/files"

/* the tree of the platform is the one the package was installed
into, nothing is ever unpacked beside the binary there */
static __inline char *get_base_path(void) {
    return (char *) VIRIATUM_DATA_PATH "/viriatum";
}

#define VIRIATUM_MODULES_PATH VIRIATUM_DATA_PATH "/viriatum/modules"
#define VIRIATUM_RESOURCES_PATH VIRIATUM_DATA_PATH "/viriatum/www"
#define VIRIATUM_CONFIG_PATH VIRIATUM_DATA_PATH "/viriatum/config"
#define VIRIATUM_BASE_PATH ""
#define VIRIATUM_LISTING_PATH "/templates/listing.html.tpl"
#define VIRIATUM_ERROR_PATH "/templates/error.html.tpl"
#define VIRIATUM_CONTENTS_PATH VIRIATUM_DATA_PATH "/viriatum/www"
#define VIRIATUM_LOG_PATH VIRIATUM_DATA_PATH "/viriatum/viriatum.log"
#define VIRIATUM_LOG_E_PATH VIRIATUM_DATA_PATH "/viriatum/viriatum.err"
#ifndef VIRIATUM_EMBED
#define VIRIATUM_EMBED
#endif
#else

static char base_path[VIRIATUM_MAX_PATH_SIZE] = {'\0'};

static __inline char *get_base_path(void) {
    size_t index;
    size_t base_path_length = 0;
#ifdef VIRIATUM_PLATFORM_MACOSX
    char *resolved;
    char launch_path[VIRIATUM_MAX_PATH_SIZE];
    uint32_t buffer_size = VIRIATUM_MAX_PATH_SIZE;
#elif defined(VIRIATUM_PLATFORM_LINUX)
    ssize_t read_size;
#endif

    /* in case the path has already been resolved gives it back, the
    executable of a process never moves while it is running */
    if(base_path[0] != '\0') { return base_path; }

    /* asks for the name of the executable, the loader answering it on
    one of the platforms and the file system of the process on the
    other, a platform that offers neither leaves the path empty and so
    goes on with the tree the binary was built with */
#ifdef VIRIATUM_PLATFORM_MACOSX
    /* the loader gives back the name the process was launched through,
    which may be a link into the tree rather than the binary of it, so
    the real one is asked for before anything is taken from it, a name
    that cannot be resolved being kept the way it came, note that the
    null based variant of the resolution is used so that the buffer is
    allocated with the proper size, the in place one requires one of at
    least PATH_MAX bytes and the runtime aborts the process otherwise */
    if(_NSGetExecutablePath(launch_path, &buffer_size) == 0) {
        resolved = realpath(launch_path, NULL);
        if(resolved != NULL && strlen(resolved) < VIRIATUM_MAX_PATH_SIZE) {
            STRCPY(base_path, VIRIATUM_MAX_PATH_SIZE, resolved);
        } else {
            STRCPY(base_path, VIRIATUM_MAX_PATH_SIZE, launch_path);
        }
        free(resolved);
        base_path_length = strlen(base_path);
    }
#elif defined(VIRIATUM_PLATFORM_LINUX)
    read_size = readlink("/proc/self/exe", base_path, VIRIATUM_MAX_PATH_SIZE - 1);
    if(read_size > 0) {
        base_path[read_size] = '\0';
        base_path_length = (size_t) read_size;
    }
#endif

    /* nothing was resolved, the caller is the one that decides what to
    do without a directory to hang the rest of the tree from */
    if(base_path_length == 0) {
        base_path[0] = '\0';
        return base_path;
    }

    /* walks the path backwards looking for the separator that closes
    the directory the executable sits in, the index counts one above
    the character it is looking at so that it never wraps around */
    for(index = base_path_length; index > 0; index--) {
        if(base_path[index - 1] != '/') { continue; }
        break;
    }

    base_path[index > 0 ? index - 1 : 0] = '\0';

    return base_path;
}

#ifndef VIRIATUM_MODULES_PATH
#define VIRIATUM_MODULES_PATH "/usr/lib/viriatum/modules"
#endif
#ifndef VIRIATUM_RESOURCES_PATH
#define VIRIATUM_RESOURCES_PATH "/var/viriatum/www"
#endif
#ifndef VIRIATUM_LOG_PATH
#define VIRIATUM_LOG_PATH "/var/log/viriatum.log"
#endif
#ifndef VIRIATUM_LOG_E_PATH
#define VIRIATUM_LOG_E_PATH "/var/log/viriatum.err"
#endif
#ifndef VIRIATUM_CONFIG_PATH
#define VIRIATUM_CONFIG_PATH "/etc/viriatum"
#endif
#define VIRIATUM_BASE_PATH ""
#define VIRIATUM_LISTING_PATH "/templates/listing.html.tpl"
#define VIRIATUM_ERROR_PATH "/templates/error.html.tpl"
#ifndef VIRIATUM_CONTENTS_PATH
#define VIRIATUM_CONTENTS_PATH "/var/viriatum/www"
#endif
#endif
#define VIRIATUM_PID_PATH "/var/run/viriatum.pid"
#define VIRIATUM_MODULE_PREFIX sizeof("libviriatum_mod_") - 1
#endif

/**
 * The path of the configuration file that a tree which was
 * unpacked rather than installed carries, taken from the
 * directory that the binary of it sits in.
 */
#define VIRIATUM_BUNDLE_CONFIG_PATH VIRIATUM_PATH_SEPARATOR "config" VIRIATUM_PATH_SEPARATOR "viriatum.ini"

#define VIRIATUM_NAME "viriatum"
#define VIRIATUM_VERSION TOSTRING(VIRIATUM_MAJOR) "." TOSTRING(VIRIATUM_MINOR) "." TOSTRING(VIRIATUM_MICRO) VIRIATUM_STAGE
#define VIRIATUM_AGENT VIRIATUM_NAME "/" VIRIATUM_VERSION
#define VIRIATUM_DESCRIPTION "Viriatum"
#define VIRIATUM_OBSERVATIONS "Viriatum HTTP Server"
#define VIRIATUM_COPYRIGHT "Copyright (c) 2008-2026 Hive Solutions Lda. All rights reserved."
#define VIRIATUM_PREFIX "VR"
#define VIRIATUM_STAGE_ALPHA "a"
#define VIRIATUM_STAGE_BETA "b"
#define VIRIATUM_STAGE_FINAL ""
#define VIRIATUM_MAJOR 0
#define VIRIATUM_MINOR 5
#define VIRIATUM_MICRO 0
#define VIRIATUM_STAGE_NAME VIRIATUM_STAGE_FINAL
#define VIRIATUM_STAGE_INDEX 0

#if VIRIATUM_STAGE_INDEX > 0
#define VIRIATUM_STAGE VIRIATUM_STAGE_NAME TOSTRING(VIRIATUM_STAGE_INDEX)
#else
#define VIRIATUM_STAGE VIRIATUM_STAGE_NAME
#endif

#ifdef VIRIATUM_DEBUG
#define VIRIATUM_DEFAULT_HOST "0.0.0.0"
#define VIRIATUM_DEFAULT_HOST6 "[::]"
#define VIRIATUM_DEFAULT_PORT 9090
#define VIRIATUM_DEFAULT_HANDLER "dispatch"
#define VIRIATUM_DEFAULT_INDEX 1
#define VIRIATUM_DEFAULT_USE_TEMPLATE 0
#define VIRIATUM_DEFAULT_HTTP2 1
#define VIRIATUM_NON_BLOCKING 1
#define VIRIATUM_KEEPALIVE 1
#define VIRIATUM_NO_WAIT 1
#define VIRIATUM_NO_PUSH 1
#define VIRIATUM_SOCKET_ERROR 0
#define VIRIATUM_SELECT_TIMEOUT 10
#define VIRIATUM_READB_SIZE 16384
#define VIRIATUM_READ_SIZE 4096
#define VIRIATUM_MAX_READ 1048576
#define VIRIATUM_TRE_READ 786432
#define VIRIATUM_MAX_EVENTS 1024
#endif

#ifndef VIRIATUM_DEBUG
#define VIRIATUM_DEFAULT_HOST "0.0.0.0"
#define VIRIATUM_DEFAULT_HOST6 "[::]"
#define VIRIATUM_DEFAULT_PORT 9090
#define VIRIATUM_DEFAULT_HANDLER "dispatch"
#define VIRIATUM_DEFAULT_INDEX 1
#define VIRIATUM_DEFAULT_USE_TEMPLATE 0
#define VIRIATUM_DEFAULT_HTTP2 1
#define VIRIATUM_NON_BLOCKING 1
#define VIRIATUM_KEEPALIVE 1
#define VIRIATUM_NO_WAIT 1
#define VIRIATUM_NO_PUSH 1
#define VIRIATUM_SOCKET_ERROR 0
#define VIRIATUM_SELECT_TIMEOUT 1
#define VIRIATUM_READB_SIZE 16384
#define VIRIATUM_READ_SIZE 4096
#define VIRIATUM_MAX_READ 1048576
#define VIRIATUM_TRE_READ 786432
#define VIRIATUM_MAX_EVENTS 1024
#endif

#ifdef VIRIATUM_DEBUG
#define VIRIATUM_DEBUG_S " debug"
#else
#define VIRIATUM_DEBUG_S ""
#endif

#ifdef VIRIATUM_THREAD_SAFE
#define VIRIATUM_THREAD_SAFE_S " ts"
#else
#define VIRIATUM_THREAD_SAFE_S " nts"
#endif

#ifdef VIRIATUM_MPOOL
#define VIRIATUM_MPOOL_S " mpool"
#else
#define VIRIATUM_MPOOL_S ""
#endif

#ifdef VIRIATUM_PREFORK
#define VIRIATUM_PREFORK_S " pfork"
#else
#define VIRIATUM_PREFORK_S ""
#endif

#ifdef VIRIATUM_EPOLL
#define VIRIATUM_EPOLL_S " epoll"
#else
#define VIRIATUM_EPOLL_S ""
#endif

#ifdef VIRIATUM_KQUEUE
#define VIRIATUM_KQUEUE_S " kqueue"
#else
#define VIRIATUM_KQUEUE_S ""
#endif

/**
 * The name of the mechanism that the service waits on its
 * connections through, the one that scales with the number
 * of them being preferred and the one that walks every one
 * of them on every pass being what is left to fall back on.
 */
#ifdef VIRIATUM_EPOLL
#define VIRIATUM_POLLING "epoll"
#elif defined(VIRIATUM_KQUEUE)
#define VIRIATUM_POLLING "kqueue"
#else
#define VIRIATUM_POLLING "select"
#endif

#ifdef VIRIATUM_IP6
#define VIRIATUM_IP6_S " ipv6"
#else
#define VIRIATUM_IP6_S ""
#endif

/**
 * The set of cipher suites that the service offers, every one of
 * them provides both forward secrecy and authenticated encryption
 * so that none of the ones that HTTP/2 refuses is ever negotiated.
 */
#define VIRIATUM_SSL_CIPHERS "ECDHE+AESGCM:ECDHE+CHACHA20:DHE+AESGCM:!aNULL:!eNULL:!MD5:!RC4:!3DES:!DSS"

#ifdef VIRIATUM_SSL
#define VIRIATUM_SSL_S " ssl"
#else
#define VIRIATUM_SSL_S ""
#endif

#ifdef VIRIATUM_PCRE
#define VIRIATUM_PCRE_S " pcre"
#else
#define VIRIATUM_PCRE_S ""
#endif

#ifdef VIRIATUM_HTTP2
#define VIRIATUM_HTTP2_S " http2"
#else
#define VIRIATUM_HTTP2_S ""
#endif

#define _VIRIATUM_FLAGS VIRIATUM_DEBUG_S VIRIATUM_THREAD_SAFE_S VIRIATUM_MPOOL_S \
    VIRIATUM_PREFORK_S VIRIATUM_EPOLL_S VIRIATUM_KQUEUE_S VIRIATUM_IP6_S         \
        VIRIATUM_SSL_S VIRIATUM_PCRE_S VIRIATUM_HTTP2_S
#define VIRIATUM_FLAGS TRIM_STRING(_VIRIATUM_FLAGS)
