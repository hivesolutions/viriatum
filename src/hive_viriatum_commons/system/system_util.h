/*
 Hive Viriatum Commons
 Copyright (C) 2008 Hive Solutions Lda.

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
 __copyright__ = Copyright (c) 2008 Hive Solutions Lda.
 __license__   = GNU General Public License (GPL), Version 3
*/

#pragma once

#ifdef VIRIATUM_PLATFORM_WIN32
#define PID_TYPE DWORD
#define LOCAL_TIME(localTimeValue, timeValue) tm localTimeValueValue; localTimeValue = &localTimeValueValue; localtime_s(localTimeValue, timeValue)
#define SLEEP(miliseconds) Sleep(miliseconds)
#define GET_PID() GetCurrentProcessId()
#define SPRINTF(buffer, size, format, ...) sprintf_s(buffer, size, format, __VA_ARGS__)
#define FOPEN(filePointer, fileName, mode) fopen_s(filePointer, fileName, mode)
#define GET_ENV(buffer, bufferSize, variableName) _dupenv_s(&buffer, &bufferSize, variableName)
#define FILE_EXISTS(filePath) GetFileAttributes(filePath) != 0xffffffff
#endif

#ifdef VIRIATUM_PLATFORM_MINGW
#define SPRINTF(buffer, size, format, ...) sprintf(buffer, format, __VA_ARGS__)
#define FOPEN(filePointer, fileName, mode) *filePointer = fopen(fileName, mode)
#endif

#ifdef VIRIATUM_PLATFORM_UNIX
#define PID_TYPE pid_t
#define LOCAL_TIME(localTimeValue, timeValue) localTimeValue = localtime(timeValue)
#define SLEEP(miliseconds) usleep((useconds_t) miliseconds * 1000)
#define GET_PID() getpid()
#define SPRINTF(buffer, size, format, ...) sprintf(buffer, format, __VA_ARGS__)
#define FOPEN(filePointer, fileName, mode) *filePointer = fopen(fileName, mode)
#define GET_ENV(buffer, bufferSize, variableName) buffer = getenv(variableName)
#define FILE_EXISTS(filePath) access(filePath, F_OK) == 0
#endif

#define CLOCK() clock()
