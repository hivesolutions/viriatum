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

VIRIATUM_EXTERNAL_PREFIX unsigned char local;

static char base_path[VIRIATUM_MAX_PATH_SIZE] = { '\0' };
static char resourcesPath[VIRIATUM_MAX_PATH_SIZE] = { '\0' };
static char modulesPath[VIRIATUM_MAX_PATH_SIZE] = { '\0' };
static char configPath[VIRIATUM_MAX_PATH_SIZE] = { '\0' };

static __inline char *getBasePath() {
    size_t base_path_length;
    size_t index;

    if(base_path[0] != '\0') { return base_path; }

    GetModuleFileName(NULL, base_path, VIRIATUM_MAX_PATH_SIZE);
    base_path_length = strlen(base_path);

    for(index = base_path_length; index >= 0; index--) {
        if(base_path[index] != '\\') { continue; }
        break;
    }

    memcpy(base_path, base_path, index);
    base_path[index] = '\0';

    return base_path;
}

static __inline char *getContentsPath() {
    const char *base_path;

    if(resourcesPath[0] != '\0') { return resourcesPath; }

    base_path = local ? "." : getBasePath();
    if(local) { sprintf_s(resourcesPath, VIRIATUM_MAX_PATH_SIZE, "%s", base_path); }
    else { sprintf_s(resourcesPath, VIRIATUM_MAX_PATH_SIZE, "%s/htdocs", base_path); }

    return resourcesPath;
}

static __inline char *getResourcesPath() {
    const char *base_path;

    if(resourcesPath[0] != '\0') { return resourcesPath; }

    base_path = getBasePath();
    sprintf_s(resourcesPath, VIRIATUM_MAX_PATH_SIZE, "%s/htdocs", base_path);

    return resourcesPath;
}

static __inline char *getModulesPath() {
    const char *base_path;

    if(modulesPath[0] != '\0') { return modulesPath; }

    base_path = getBasePath();
    sprintf_s(modulesPath, VIRIATUM_MAX_PATH_SIZE, "%s/modules", base_path);

    return modulesPath;
}

static __inline char *getConfigPath() {
    const char *base_path;

    if(configPath[0] != '\0') { return configPath; }

    base_path = getBasePath();
    sprintf_s(configPath, VIRIATUM_MAX_PATH_SIZE, "%s/config", base_path);

    return configPath;
}

#ifndef VIRIATUM_MODULES_PATH
#define VIRIATUM_MODULES_PATH getModulesPath()
#endif
#define VIRIATUM_RESOURCES_PATH getResourcesPath()
#define VIRIATUM_CONFIG_PATH getConfigPath()
#define VIRIATUM_BASE_PATH ""
#define VIRIATUM_LISTING_PATH "/templates/listing.html.tpl"
#ifndef VIRIATUM_CONTENTS_PATH
#define VIRIATUM_CONTENTS_PATH getContentsPath()
#endif
#define VIRIATUM_PID_PATH "viriatum.pid"
#endif

#ifdef VIRIATUM_PLATFORM_UNIX
#ifdef VIRIATUM_PLATFORM_ANDROID
#define VIRIATUM_MODULES_PATH "/sdcard/viriatum/modules"
#define VIRIATUM_RESOURCES_PATH "/sdcard/viriatum/www"
#define VIRIATUM_CONFIG_PATH "/sdcard/viriatum/config"
#define VIRIATUM_BASE_PATH ""
#define VIRIATUM_LISTING_PATH "/templates/listing.html.tpl"
#define VIRIATUM_CONTENTS_PATH "/sdcard/viriatum/www"
#else
#ifndef VIRIATUM_MODULES_PATH
#define VIRIATUM_MODULES_PATH "/usr/lib/viriatum/modules"
#endif
#ifndef VIRIATUM_RESOURCES_PATH
#define VIRIATUM_RESOURCES_PATH "/var/viriatum/www"
#endif
#ifndef VIRIATUM_CONFIG_PATH
#define VIRIATUM_CONFIG_PATH "/etc/viriatum"
#endif
#define VIRIATUM_BASE_PATH ""
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
#define VIRIATUM_PREFIX "VR"
#define VIRIATUM_MAJOR 0
#define VIRIATUM_MINOR 1
#define VIRIATUM_MICRO 0

#ifdef VIRIATUM_DEBUG
#define VIRIATUM_DEFAULT_HOST "0.0.0.0"
#define VIRIATUM_DEFAULT_PORT 9090
#define VIRIATUM_DEFAULT_HANDLER "file"
#define VIRIATUM_DEFAULT_INDEX 1
#define VIRIATUM_NON_BLOCKING 1
#define VIRIATUM_SOCKET_ERROR 0
#define VIRIATUM_SELECT_TIMEOUT 10
#define VIRIATUM_MAXIMUM_CONNECTIONS 10000
#endif

#ifndef VIRIATUM_DEBUG
#define VIRIATUM_DEFAULT_HOST "0.0.0.0"
#define VIRIATUM_DEFAULT_PORT 9090
#define VIRIATUM_DEFAULT_HANDLER "file"
#define VIRIATUM_DEFAULT_INDEX 1
#define VIRIATUM_NON_BLOCKING 1
#define VIRIATUM_SOCKET_ERROR 0
#define VIRIATUM_SELECT_TIMEOUT 1
#define VIRIATUM_MAXIMUM_CONNECTIONS 10000
#endif
