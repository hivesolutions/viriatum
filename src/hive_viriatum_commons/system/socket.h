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
#include <winsock.h>
#pragma comment(lib, "Ws2_32.lib")
#define SOCKET_DATA WSADATA
#define SOCKET_HANDLE SOCKET
#define SOCKET_ADDRESS SOCKADDR
#define SOCKET_ADDRESS_INPUT SOCKADDR_IN
#define SOCKET_ERROR_CODE int
#define SOCKET_INTERNET_TYPE AF_INET
#define SOCKET_INTERNET6_TYPE AF_INET6
#define SOCKET_PACKET_TYPE SOCK_STREAM
#define SOCKET_DATAGRAM_TYPE SOCK_DGRAM
#define SOCKET_PROTOCOL_TCP IPPROTO_TCP
#define SOCKET_PROTOCOL_UDP IPPROTO_UDP
#define SOCKET_INITIALIZE(socketData) WSAStartup(MAKEWORD(2, 0), socketData)
#define SOCKET_FINISH() WSACleanup()
#define SOCKET_CREATE(type, streamType, protocolType) socket(type, streamType, protocolType)
#define SOCKET_BIND(socketHandle, socketAddress) bind(socketHandle, (LPSOCKADDR) &socketAddress, sizeof(SOCKET_ADDRESS))
#define SOCKET_LISTEN(socketHandle) listen(socketHandle, SOMAXCONN)
#define SOCKET_CONNECT(socketHandle, socketAddress) connect(socketHandle, (LPSOCKADDR) &socketAddress, sizeof(SOCKET_ADDRESS))
#define SOCKET_ACCEPT(socketHandle, socketAddress, socketAddressSize) accept(socketHandle, socketAddress, &socketAddressSize)
#define SOCKET_CLOSE(socketHandle) closesocket(socketHandle)
#define SOCKET_ADDRESS_CREATE(socketAddress, type, address, port) memset(&socketAddress, 0, sizeof(SOCKET_ADDRESS));\
    socketAddress.sin_family = type;\
    socketAddress.sin_port = htons(port);\
    if(address == NULL) { socketAddress.sin_addr.s_addr = INADDR_ANY; } else { socketAddress.sin_addr.s_addr = inet_addr(address); }
#define SOCKET_GET_HOST_BY_NAME(hostname) gethostbyname(hostname)
#define SOCKET_TEST_SOCKET(socketHandle) socketHandle == INVALID_SOCKET
#define SOCKET_TEST_ERROR(result) result == SOCKET_ERROR
#define SOCKET_GET_ERROR_CODE(result) WSAGetLastError()
#define SOCKET_SEND(socketHandle, buffer, length, flags) send(socketHandle, buffer, length, flags)
#define SOCKET_RECEIVE(socketHandle, buffer, length, flags) recv(socketHandle, buffer, length, flags)
#define SOCKET_SET_OPTIONS(socketHandle, level, optionName, optionValue) setsockopt(socketHandle, level, optionName, &optionValue, sizeof(optionValue))
#endif

#ifdef VIRIATUM_PLATFORM_UNIX
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define SOCKET_HANDLE int
#define SOCKET_DATA void *
#define SOCKET_ADDRESS struct sockaddr
#define SOCKET_ADDRESS_INPUT struct sockaddr_in
#define SOCKET_ERROR_CODE int
#define SOCKET_INTERNET_TYPE AF_INET
#define SOCKET_INTERNET6_TYPE AF_INET6
#define SOCKET_PACKET_TYPE SOCK_STREAM
#define SOCKET_DATAGRAM_TYPE SOCK_DGRAM
#define SOCKET_PROTOCOL_TCP IPPROTO_TCP
#define SOCKET_PROTOCOL_UDP IPPROTO_UDP
#define SOCKET_INITIALIZE(socketData)
#define SOCKET_FINISH()
#define SOCKET_CREATE(type, streamType, protocolType) socket(type, streamType, protocolType)
#define SOCKET_BIND(socketHandle, socketAddress) bind(socketHandle, (struct sockaddr *) &socketAddress, sizeof(SOCKET_ADDRESS))
#define SOCKET_LISTEN(socketHandle) listen(socketHandle, SOMAXCONN)
#define SOCKET_CONNECT(socketHandle, socketAddress) connect(socketHandle, (struct sockaddr *) &socketAddress, sizeof(SOCKET_ADDRESS))
#define SOCKET_ACCEPT(socketHandle, socketAddress, socketAddressSize) accept(socketHandle, socketAddress, &socketAddressSize)
#define SOCKET_CLOSE(socketHandle) close(socketHandle)
#define SOCKET_ADDRESS_CREATE(socketAddress, type, address, port) memset(&socketAddress, 0, sizeof(SOCKET_ADDRESS));\
    socketAddress.sin_family = type;\
    socketAddress.sin_port = htons(port);\
    if(address == NULL) { socketAddress.sin_addr.s_addr = INADDR_ANY; } else { socketAddress.sin_addr.s_addr = inet_addr(address); }
#define SOCKET_GET_HOST_BY_NAME(hostname) gethostbyname(hostname)
#define SOCKET_TEST_SOCKET(socketHandle) socketHandle < 0
#define SOCKET_TEST_ERROR(result) result < 0
#define SOCKET_GET_ERROR_CODE(result) result
#define SOCKET_SEND(socketHandle, buffer, length, flags) write(socketHandle, buffer, length)
#define SOCKET_RECEIVE(socketHandle, buffer, length, flags) read(socketHandle, buffer, length)
#define SOCKET_SET_OPTIONS(socketHandle, level, optionName, optionValue) setsockopt(socketHandle, level, optionName, &optionValue, sizeof(optionValue))
#endif
