/*
 Hive Viriatum Web Server
 Copyright (C) 2008 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#pragma once

#ifdef VIRIATUM_PLATFORM_WIN32

static char basePath[VIRIATUM_MAX_PATH_SIZE] = { '\0' };
static char modulesPath[VIRIATUM_MAX_PATH_SIZE] = { '\0' };
static char configPath[VIRIATUM_MAX_PATH_SIZE] = { '\0' };

static __inline char *getBasePath() {
    size_t basePathLength;
    size_t index;

    if(basePath[0] != '\0') { return basePath; }

    GetModuleFileName(NULL, basePath, VIRIATUM_MAX_PATH_SIZE);
    basePathLength = strlen(basePath);

    for(index = basePathLength; index >= 0; index--) {
        if(basePath[index] != '\\') { continue; }
        break;
    }

    memcpy(basePath, basePath, index);
    basePath[index] = '\0';

    return basePath;
}

static __inline char *getModulesPath() {
    const char *basePath;

    if(modulesPath[0] != '\0') { return modulesPath; }

    basePath = getBasePath();
    sprintf_s(modulesPath, VIRIATUM_MAX_PATH_SIZE, "%s/modules", basePath);

    return modulesPath;
}

static __inline char *getConfigPath() {
    const char *basePath;

    if(configPath[0] != '\0') { return configPath; }

    basePath = getBasePath();
    sprintf_s(configPath, VIRIATUM_MAX_PATH_SIZE, "%s/config", basePath);

    return configPath;
}

#ifndef VIRIATUM_MODULES_PATH
#define VIRIATUM_MODULES_PATH getModulesPath()
#endif
#define VIRIATUM_RESOURCES_PATH getBasePath()
#define VIRIATUM_CONFIG_PATH getConfigPath()
#define VIRIATUM_BASE_PATH "/"
#define VIRIATUM_LISTING_PATH "/templates/listing.html.tpl"
#ifndef VIRIATUM_CONTENTS_PATH
#define VIRIATUM_CONTENTS_PATH "."
#endif
#define VIRIATUM_PID_PATH "viriatum.pid"
#endif

#ifdef VIRIATUM_PLATFORM_UNIX
#ifdef VIRIATUM_PLATFORM_ANDROID
#define VIRIATUM_MODULES_PATH "/sdcard/viriatum/modules"
#define VIRIATUM_RESOURCES_PATH "/sdcard/viriatum/www"
#define VIRIATUM_CONFIG_PATH "/sdcard/viriatum/config"
#define VIRIATUM_BASE_PATH "/"
#define VIRIATUM_LISTING_PATH "/templates/listing.html.tpl"
#define VIRIATUM_CONTENTS_PATH "/sdcard/viriatum/www"
#else
#ifndef VIRIATUM_MODULES_PATH
#define VIRIATUM_MODULES_PATH "/usr/lib/viriatum/modules"
#endif
#define VIRIATUM_RESOURCES_PATH "/var/viriatum/www"
#define VIRIATUM_CONFIG_PATH "/etc/viriatum"
#define VIRIATUM_BASE_PATH "/"
#define VIRIATUM_LISTING_PATH "/templates/listing.html.tpl"
#ifndef VIRIATUM_CONTENTS_PATH
#define VIRIATUM_CONTENTS_PATH "/var/viriatum/www"
#endif
#endif
#define VIRIATUM_PID_PATH "/var/run/viriatum.pid"
#endif

#define VIRIATUM_NAME "viriatum"
#define VIRIATUM_VERSION "0.1.0"
#define VIRIATUM_DESCRIPTION "Viriatum"
#define VIRIATUM_OBSERVATIONS "Viriatum HTTP Server"
#define VIRIATUM_COPYRIGHT "Copyright (c) 2010 Hive Solutions Lda. All rights reserved."

#ifdef VIRIATUM_DEBUG
#define VIRIATUM_DEFAULT_HOST "0.0.0.0"
#define VIRIATUM_DEFAULT_PORT 9090
#define VIRIATUM_DEFAULT_HANDLER "file"
#define VIRIATUM_DEFAULT_INDEX 0
#define VIRIATUM_NON_BLOCKING 1
#define VIRIATUM_SOCKET_ERROR 0
#define VIRIATUM_SELECT_TIMEOUT 10
#define VIRIATUM_MAXIMUM_CONNECTIONS 10000
#endif

#ifndef VIRIATUM_DEBUG
#define VIRIATUM_DEFAULT_HOST "0.0.0.0"
#define VIRIATUM_DEFAULT_PORT 9090
#define VIRIATUM_DEFAULT_HANDLER "file"
#define VIRIATUM_DEFAULT_INDEX 0
#define VIRIATUM_NON_BLOCKING 1
#define VIRIATUM_SOCKET_ERROR 0
#define VIRIATUM_SELECT_TIMEOUT 1
#define VIRIATUM_MAXIMUM_CONNECTIONS 10000
#endif
