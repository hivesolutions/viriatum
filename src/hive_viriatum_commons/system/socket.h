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
#ifdef VIRIATUM_IP6
#include <Ws2tcpip.h>
#else
#include <winsock.h>
#endif
#ifdef VIRIATUM_IP6
#define SOCKET_ADDRESS SOCKADDR_STORAGE
#define SOCKET_ADDRESS_INTERNET6 SOCKET_INFO
#else
#define SOCKET_ADDRESS SOCKADDR
#endif
#define SOCKET_CONNECTIONS 256
#define SOCKET_DATA WSADATA
#define SOCKET_INFO ADDRINFO
#define SOCKET_HANDLE SOCKET
#define SOCKET_ADDRESS_SIZE int
#define SOCKET_ADDRESS_INTERNET SOCKADDR_IN
#define SOCKET_HOSTENT HOSTENT
#define SOCKET_FLAGS unsigned long
#define SOCKET_OPTION char
#define SOCKET_OPTION_LARGE int
#define SOCKET_ERROR_CODE int
#define SOCKET_IOCTL ioctlsocket
#define SOCKET_WOULDBLOCK WSAEWOULDBLOCK
#define SOCKET_INTERNET_TYPE AF_INET
#define SOCKET_INTERNET6_TYPE AF_INET6
#define SOCKET_INTERNET_ALL_TYPE PF_UNSPEC
#define SOCKET_PACKET_TYPE SOCK_STREAM
#define SOCKET_DATAGRAM_TYPE SOCK_DGRAM
#define SOCKET_PROTOCOL_TCP IPPROTO_TCP
#define SOCKET_PROTOCOL_UDP IPPROTO_UDP
#define SOCKET_OPTIONS_LEVEL_SOCKET SOL_SOCKET
#define SOCKET_OPTIONS_IP6_SOCKET IPPROTO_IPV6
#define SOCKET_OPTIONS_REUSE_ADDRESS_SOCKET SO_REUSEADDR
#define SOCKET_OPTIONS_IP6_ONLY_SOCKET IPV6_V6ONLY
#define SOCKET_SET fd_set
#define SOCKET_INITIALIZE(socket_data) WSAStartup(MAKEWORD(2, 0), socket_data)
#define SOCKET_FINISH() WSACleanup()
#define SOCKET_CREATE(type, stream_type, protocol_type) socket(type, stream_type, protocol_type)
#define SOCKET_BIND(socket_handle, socket_address) bind(socket_handle, (LPSOCKADDR) &socket_address, sizeof(SOCKET_ADDRESS_INTERNET))
#define SOCKET_BIND_EX(socket_handle, socket_address, socket_address_size) bind(socket_handle, (LPSOCKADDR) socket_address, socket_address_size)
#define SOCKET_LISTEN(socket_handle) listen(socket_handle, SOCKET_CONNECTIONS)
#define SOCKET_CONNECT(socket_handle, socket_address) connect(socket_handle, (LPSOCKADDR) &socket_address, sizeof(SOCKET_ADDRESS))
#define SOCKET_CONNECT_SIZE(socket_handle, socket_address, socket_address_size) connect(socket_handle, (LPSOCKADDR) &socket_address, socket_address_size)
#define SOCKET_ACCEPT(socket_handle, socket_address, socket_address_size) accept(socket_handle, (LPSOCKADDR) socket_address, &socket_address_size)
#define SOCKET_CLOSE(socket_handle) closesocket(socket_handle)
#define SOCKET_ADDRESS_CREATE(socket_address, type, address, port) memset(&socket_address, 0, sizeof(SOCKET_ADDRESS));\
    socket_address.sin_family = type;\
    socket_address.sin_port = htons(port);\
    if(address == NULL) { socket_address.sin_addr.s_addr = INADDR_ANY; } else { socket_address.sin_addr.s_addr = inet_addr(address); }
#define SOCKET_GET_HOST_BY_NAME(hostname) gethostbyname(hostname)
#define SOCKET_TEST_SOCKET(socket_handle) socket_handle == INVALID_SOCKET
#define SOCKET_TEST_ERROR(result) result == SOCKET_ERROR
#define SOCKET_EX_TEST_ERROR(result) result == SOCKET_ERROR || result == 0
#define SOCKET_GET_ERROR_CODE(result) WSAGetLastError()
#define SOCKET_SEND(socket_handle, buffer, length, flags) send(socket_handle, buffer, length, flags)
#define SOCKET_RECEIVE(socket_handle, buffer, length, flags) recv(socket_handle, buffer, length, flags)
#define SOCKET_SET_OPTIONS(socket_handle, level, option_name, option_value) setsockopt(socket_handle, level, option_name, (SOCKET_OPTION *) &option_value, sizeof(option_value))
#define SOCKET_SELECT(number_socket, sockets_read_set, sockets_write_set, sockets_exception_set, timeout) select(number_socket, sockets_read_set, sockets_write_set, sockets_exception_set, timeout)
#define SOCKET_SET_ZERO(sockets_set) FD_ZERO(sockets_set)
#define SOCKET_SET_SET(socket_handle, sockets_set) FD_SET(socket_handle, sockets_set)
#define SOCKET_SET_CLEAR(socket_handle, sockets_set) FD_CLR(socket_handle, sockets_set)
#define SOCKET_SET_IS_SET(socket_handle, sockets_set) FD_ISSET(socket_handle, sockets_set)
#define SOCKET_SET_NON_BLOCKING(socket_handle, flags) SOCKET_IOCTL(socket_handle, FIONBIO, &flags)
#define SOCKET_SET_NO_WAIT(socket_handle, option_value) option_value = 1; SOCKET_SET_OPTIONS(socket_handle, SOCKET_PROTOCOL_TCP, TCP_NODELAY, option_value)
#define SOCKET_SET_NO_PUSH(socket_handle, option_value)
#endif

#ifdef VIRIATUM_PLATFORM_UNIX
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#ifdef VIRIATUM_IP6
#define SOCKET_ADDRESS_INTERNET6 struct sockaddr_in6
#endif
#define SOCKET_CONNECTIONS 256
#define SOCKET_DATA void *
#define SOCKET_HANDLE int
#define SOCKET_ADDRESS_SIZE socklen_t
#define SOCKET_ADDRESS struct sockaddr_in6
#define SOCKET_ADDRESS_INTERNET struct sockaddr_in
#define SOCKET_HOSTENT struct hostent
#define SOCKET_FLAGS unsigned long
#define SOCKET_OPTION int
#define SOCKET_OPTION_LARGE int
#define SOCKET_ERROR_CODE int
#define SOCKET_IOCTL ioctl
#define SOCKET_FCNTL fcntl
#define SOCKET_WOULDBLOCK EWOULDBLOCK
#define SOCKET_INTERNET_TYPE AF_INET
#define SOCKET_INTERNET6_TYPE AF_INET6
#define SOCKET_PACKET_TYPE SOCK_STREAM
#define SOCKET_DATAGRAM_TYPE SOCK_DGRAM
#define SOCKET_PROTOCOL_TCP IPPROTO_TCP
#define SOCKET_PROTOCOL_UDP IPPROTO_UDP
#define SOCKET_OPTIONS_LEVEL_SOCKET SOL_SOCKET
#define SOCKET_OPTIONS_IP6_SOCKET IPPROTO_IPV6
#define SOCKET_OPTIONS_REUSE_ADDRESS_SOCKET SO_REUSEADDR
#define SOCKET_OPTIONS_IP6_ONLY_SOCKET IPV6_V6ONLY
#define SOCKET_SET fd_set
#define SOCKET_INITIALIZE(socket_data) dump((void *) socket_data)
#define SOCKET_FINISH()
#define SOCKET_CREATE(type, stream_type, protocol_type) socket(type, stream_type, protocol_type)
#define SOCKET_BIND(socket_handle, socket_address) bind(socket_handle, (struct sockaddr *) &socket_address, sizeof(SOCKET_ADDRESS_INTERNET))
#define SOCKET_BIND_EX(socket_handle, socket_address, socket_address_size) bind(socket_handle, (struct sockaddr *) &socket_address, socket_address_size)
#define SOCKET_LISTEN(socket_handle) listen(socket_handle, SOCKET_CONNECTIONS)
#define SOCKET_CONNECT(socket_handle, socket_address) connect(socket_handle, (struct sockaddr *) &socket_address, sizeof(SOCKET_ADDRESS))
#define SOCKET_CONNECT_SIZE(socket_handle, socket_address, socket_address_size) connect(socket_handle, (struct sockaddr *) &socket_address, socket_address_size)
#define SOCKET_ACCEPT(socket_handle, socket_address, socket_address_size) accept(socket_handle, (struct sockaddr *) socket_address, &socket_address_size)
#define SOCKET_CLOSE(socket_handle) close(socket_handle)
#define SOCKET_ADDRESS_CREATE(socket_address, type, address, port) memset(&socket_address, 0, sizeof(SOCKET_ADDRESS));\
    socket_address.sin_family = type;\
    socket_address.sin_port = htons(port);\
    if(address == NULL) { socket_address.sin_addr.s_addr = INADDR_ANY; } else { socket_address.sin_addr.s_addr = inet_addr(address); }
#define SOCKET_GET_HOST_BY_NAME(hostname) gethostbyname(hostname)
#define SOCKET_TEST_SOCKET(socket_handle) socket_handle < 0
#define SOCKET_TEST_ERROR(result) result < 0
#define SOCKET_EX_TEST_ERROR(result) result <= 0
#define SOCKET_GET_ERROR_CODE(result) (SOCKET_ERROR_CODE) errno
#define SOCKET_SEND(socket_handle, buffer, length, flags) write(socket_handle, buffer, length)
#define SOCKET_RECEIVE(socket_handle, buffer, length, flags) read(socket_handle, buffer, length)
#define SOCKET_SET_OPTIONS(socket_handle, level, option_name, option_value) setsockopt(socket_handle, level, option_name, (SOCKET_OPTION *) &option_value, sizeof(option_value))
#define SOCKET_SELECT(number_socket, sockets_read_set, sockets_write_set, sockets_exception_set, timeout) select(number_socket, sockets_read_set, sockets_write_set, sockets_exception_set, timeout)
#define SOCKET_SET_ZERO(sockets_set) FD_ZERO(sockets_set)
#define SOCKET_SET_SET(socket_handle, sockets_set) FD_SET(socket_handle, sockets_set)
#define SOCKET_SET_CLEAR(socket_handle, sockets_set) FD_CLR(socket_handle, sockets_set)
#define SOCKET_SET_IS_SET(socket_handle, sockets_set) FD_ISSET(socket_handle, sockets_set)
#define SOCKET_SET_NON_BLOCKING(socket_handle, flags) if((flags = SOCKET_FCNTL(socket_handle, F_GETFL, 0) == -1)) { flags = 0; }\
    SOCKET_FCNTL(socket_handle, F_SETFL, flags | O_NONBLOCK)
#define SOCKET_SET_NO_WAIT(socket_handle, option_value) option_value = 1; SOCKET_SET_OPTIONS(socket_handle, SOCKET_PROTOCOL_TCP, TCP_NODELAY, option_value)
#ifdef TCP_CORK
#define TCP_NOPUSH TCP_CORK
#endif
#ifdef TCP_NOPUSH
#define SOCKET_SET_NO_PUSH(socket_handle, option_value) option_value = 0; SOCKET_SET_OPTIONS(socket_handle, SOCKET_PROTOCOL_TCP, TCP_NOPUSH, option_value)
#else
#define SOCKET_SET_NO_PUSH(socket_handle, option_value)
#endif
#endif

#ifdef VIRIATUM_PLATFORM_MSC
#pragma comment(lib, "Ws2_32.lib")
#endif
