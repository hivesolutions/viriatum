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
#ifndef F_OK
#define F_OK 00
#endif
#ifndef W_OK
#define W_OK 02
#endif
#ifndef R_OK
#define R_OK 04
#endif
#ifndef X_OK
#define X_OK 06
#endif
#define PID_TYPE DWORD
#define PROCESS_TYPE HANDLE
#define MEMORY_INFORMATION_TYPE PROCESS_MEMORY_COUNTERS
#define LOCAL_TIME(local_time_value, time_value) struct tm local_time_value_value; local_time_value = &local_time_value_value; localtime_s(local_time_value, time_value)
#define SLEEP(miliseconds) Sleep(miliseconds)
#define GET_PID() GetCurrentProcessId()
#define GET_ENV(buffer, buffer_size, variable_name) _dupenv_s(&buffer, &buffer_size, variable_name)
#define GET_PROCESS() OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, GET_PID())
#define CLOSE_PROCESS(process) CloseHandle(process)
#define GET_MEMORY_INFORMATION(process, memory_information) GetProcessMemoryInfo(process, &memory_information, sizeof(memory_information))
#define GET_MEMORY_USAGE(memory_information) memory_information.PagefileUsage
#define FILE_EXISTS(file_path) GetFileAttributes(file_path) != 0xffffffff
#define ACCESS(path, mode) _access(path, mode)
#define ROUND(value) floor(value + 0.5f)
#endif

#ifdef VIRIATUM_PLATFORM_UNIX
#define PID_TYPE pid_t
#define PROCESS_TYPE int
#define MEMORY_INFORMATION_TYPE struct rusage
#define LOCAL_TIME(local_time_value, time_value) local_time_value = localtime(time_value)
#define SLEEP(miliseconds) usleep((useconds_t) miliseconds * 1000)
#define GET_PID() getpid()
#define SPRINTF(buffer, size, format, ...) sprintf(buffer, format, __VA_ARGS__)
#define SSCANF(buffer, format, ...) sscanf(buffer, format, __VA_ARGS__)
#define STRTOK(string, delimiter, context) strtok(string, delimiter); dump((void *) &context)
#define STRCPY(destination, size, source) strcpy(destination, source)
#define FOPEN(file_pointer, file_name, mode) *file_pointer = fopen(file_name, mode)
#define GET_ENV(buffer, buffer_size, variable_name) buffer = getenv(variable_name)
#define GET_PROCESS() RUSAGE_SELF
#define CLOSE_PROCESS(process)
#define GET_MEMORY_INFORMATION(process, memory_information) getrusage(process, &memory_information)
#define GET_MEMORY_USAGE(memory_information) memory_information.ru_ixrss
#define FILE_EXISTS(file_path) access(file_path, F_OK) == 0
#define ACCESS(path, mode) access(path, mode)
#define ROUND(value) floor(value + 0.5f)
#endif

#ifdef VIRIATUM_PLATFORM_MSC
#define SPRINTF(buffer, size, format, ...) sprintf_s(buffer, size, format, __VA_ARGS__)
#define SSCANF(buffer, format, ...) sscanf_s(buffer, format, __VA_ARGS__)
#define STRTOK(string, delimiter, context) strtok_s(string, delimiter, &context)
#define STRCPY(destination, size, source) strcpy_s(destination, size, source)
#define FOPEN(file_pointer, file_name, mode) fopen_s(file_pointer, file_name, mode)
#endif

#ifdef VIRIATUM_PLATFORM_MINGW
#define SPRINTF(buffer, size, format, ...) sprintf(buffer, format, __VA_ARGS__)
#define SSCANF(buffer, format, ...) sscanf(buffer, format, __VA_ARGS__)
#define STRTOK(string, delimiter, context) strtok(string, delimiter); dump((void *) &context)
#define STRCPY(destination, size, source) strcpy(destination, source)
#define FOPEN(file_pointer, file_name, mode) *file_pointer = fopen(file_name, mode)
#endif

#define CLOCK() clock()

#ifdef VIRIATUM_PLATFORM_MSC
#pragma comment(lib, "Psapi.lib")
#endif
