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

#include "stdafx.h"

#include "stream_io.h"

void createIoConnection(struct IoConnection_t **ioConnectionPointer, struct Connection_t *connection) {
    /* retrieves the io connection size */
    size_t ioConnectionSize = sizeof(struct IoConnection_t);

    /* allocates space for the io connection */
    struct IoConnection_t *ioConnection = (struct IoConnection_t *) MALLOC(ioConnectionSize);

    /* sets the io connection connection */
    ioConnection->connection = connection;

    /* sets the on data to unset */
    ioConnection->onData = NULL;

    /* sets the on open to unset */
    ioConnection->onOpen = NULL;

    /* sets the on close to unset */
    ioConnection->onClose = NULL;

    /* sets the lower to unset */
    ioConnection->lower = NULL;

    /* sets the io connection in the (upper) connection substrate */
    connection->lower = (void *) ioConnection;

    /* sets the io connection in the io connection pointer */
    *ioConnectionPointer = ioConnection;
}

void deleteIoConnection(struct IoConnection_t *ioConnection) {
    /* releases the io connection */
    FREE(ioConnection);
}

ERROR_CODE acceptHandlerStreamIo(struct Connection_t *connection) {
    /* allocates the socket handle */
    SOCKET_HANDLE socketHandle;

    /* allocates the socket address */
    SOCKET_ADDRESS socketAddress;

    /* allocates the (client) connection */
    struct Connection_t *clientConnection;

    /* sets the flags to be used in socket */
    SOCKET_FLAGS flags = 1;

    /* calculates the size of the socket address */
    SOCKET_ADDRESS_SIZE clientSocketAddressSize = sizeof(SOCKET_ADDRESS);

    /* retrieves the io connection */
    struct IoConnection_t *ioConnection = (struct IoConnection_t *) connection->lower;

    /* iterates continuously */
    while(1) {
        /* accepts the socket, retrieving the socket handle */
        socketHandle = SOCKET_ACCEPT(connection->socketHandle, &socketAddress, clientSocketAddressSize);

        /* in case there was an error accepting the socket */
        if(SOCKET_TEST_ERROR(socketHandle)) {
            /* breaks the loop */
            break;
        }
        /* otherwise the socket was accepted corretly */
        else {
            /* in case viriatum is set to non blocking */
            if(VIRIATUM_NON_BLOCKING) {
                /* sets the socket to non blocking mode */
                SOCKET_SET_NON_BLOCKING(socketHandle, flags);
            }

            /* prints a debug message */
            V_DEBUG_F("Accepted connection: %d\n", socketHandle);

            /* creates the (client) connection */
            createConnection(&clientConnection, socketHandle);

            /* sets the service select service as the service in the (client)  connection */
            clientConnection->service = connection->service;

            /* sets the base hanlding functions in the client connection */
            clientConnection->openConnection = openConnection;
            clientConnection->closeConnection = closeConnection;
            clientConnection->registerWrite = registerWriteConnection;
            clientConnection->unregisterWrite = unregisterWriteConnection;

            /* sets the various stream io connection callbacks
            in the client connection */
            clientConnection->onRead = readHandlerStreamIo;
            clientConnection->onWrite = writeHandlerStreamIo;
            clientConnection->onError = errorHandlerStreamIo;
            clientConnection->onOpen = openHandlerStreamIo;
            clientConnection->onClose = closeHandlerStreamIo;

            /* opens the connection */
            clientConnection->openConnection(clientConnection);
        }

        /* in case viriatum is set to blocking */
        if(!VIRIATUM_NON_BLOCKING) {
            /* breaks the loop (avoid blocking) */
            break;
        }
    }

    return 0;
}

ERROR_CODE readHandlerStreamIo(struct Connection_t *connection) {
    /* allocates the number of bytes */
    SOCKET_ERROR_CODE numberBytes;

    /* allocates the "simple" buffer */
    unsigned char buffer[10240];

    /* retrieves the buffer pointer */
    unsigned char *bufferPointer = (unsigned char *) buffer;

    /* allocates the buffer size */
    size_t bufferSize = 0;

    /* flag and value controlling the state of the read */
    ERROR_CODE error = 0;

    /* retrieves the io connection */
    struct IoConnection_t *ioConnection = (struct IoConnection_t *) connection->lower;

    /* iterates continuously */
    while(1) {
        /* in case the buffer size is so big that may
        overflow the current allocated buffer */
        if(bufferSize + 1024 > 10240) {
            /* breaks the loop */
            break;
        }

        /* receives from the socket */
        numberBytes = SOCKET_RECEIVE(connection->socketHandle, bufferPointer, 1024, 0);

        /* in case the number of bytes is zero (connection closed) */
        if(numberBytes == 0) {
            /* sets the error flag */
            error = 1;

            /* breaks the loop */
            break;
        }

        /* in case there was an error receiving from the socket */
        if(SOCKET_TEST_ERROR(numberBytes)) {
            /* retrieves the receving error code */
            SOCKET_ERROR_CODE receivingErrorCode = SOCKET_GET_ERROR_CODE(numberBytes);

            /* switches over the receiving error code */
            switch(receivingErrorCode) {
                case SOCKET_WOULDBLOCK:
                    /* prints a debug message */
                    V_DEBUG("Read structures not ready: WOULDBLOCK\n");

                    /* sets the error flag (non fatal) */
                    error = 2;

                    /* breaks the switch */
                    break;

                default:
                    /* prints the error */
                    V_DEBUG_F("Problem receiving from socket: %d\n", receivingErrorCode);

                    /* sets the error flag */
                    error = 1;

                    /* breaks the switch */
                    break;
            }

            /* breaks the loop */
            break;
        }

        /* increments the buffer position */
        bufferPointer += numberBytes;

        /* increments the buffer size with the number of bytes */
        bufferSize += numberBytes;

        /* in case the viriatum is set to blocking */
        if(!VIRIATUM_NON_BLOCKING) {
            /* breaks the loop */
            break;
        }
    }

    /* switches over the error flag and value */
    switch(error) {
        /* in case there's no error */
        case 0:
            /* in case the on data handler is defined */
            if(ioConnection->onData != NULL) {
                /* prints a debug message */
                V_DEBUG("Calling on data handler\n");

                /* calls the on data handler */
                ioConnection->onData(ioConnection, buffer, bufferSize);

                /* prints a debug message */
                V_DEBUG("Finished calling on data handler\n");
            }

            /* breaks the switch */
            break;

        /* in case it's a fatal error */
        case 1:
            /* closes the connection */
            connection->closeConnection(connection);

            /* breaks the switch */
            break;

        /* in case it's a non fatal error */
        case 2:
            /* in case the on data handler is defined */
            if(ioConnection->onData != NULL) {
                /* prints a debug message */
                V_DEBUG("Calling on data handler\n");

                /* calls the on data handler */
                ioConnection->onData(ioConnection, buffer, bufferSize);

                /* prints a debug message */
                V_DEBUG("Finished calling on data handler\n");
            }

            /* breaks the switch */
            break;

        /* default case */
        default:
            /* breaks the switch */
            break;
    }

    /* returns the error code */
    return error;
}

ERROR_CODE writeHandlerStreamIo(struct Connection_t *connection) {
    /* allocates the number of bytes */
    SOCKET_ERROR_CODE numberBytes;

    /* allocates the data */
    struct Data_t *data;

    /* flag and value controlling the state of the write */
    ERROR_CODE error = 0;

    /* iterates continuously */
    while(1) {
        /* prints a debug message */
        V_DEBUG("Peeking value from write queue\n");

        /* peeks a value (data) from the linked list (write queue) */
        peekValueLinkedList(connection->writeQueue, (void **) &data);

        /* in case the data is invalid (list is empty) */
        if(data == NULL) {
            /* breaks the loop */
            break;
        }

        /* prints a debug message */
        V_DEBUG_F("Sending %d bytes through socket: %d\n", data->size, connection->socketHandle);

        /* sends the value retrieving the number of bytes sent */
        numberBytes = SOCKET_SEND(connection->socketHandle, data->data, data->size, 0);

        /* in case there was an error receiving from the socket */
        if(SOCKET_TEST_ERROR(numberBytes)) {
            /* retrieves the receving error code */
            SOCKET_ERROR_CODE receivingErrorCode = SOCKET_GET_ERROR_CODE(numberBytes);

            /* switches over the receiving error code */
            switch(receivingErrorCode) {
                case SOCKET_WOULDBLOCK:
                    /* prints a debug message */
                    V_DEBUG("Write structures not ready: WOULDBLOCK\n");

                    /* sets the error flag (non fatal) */
                    error = 2;

                    /* breaks the switch */
                    break;

                default:
                    /* prints the error */
                    V_DEBUG_F("Problem sending from socket: %d\n", receivingErrorCode);

                    /* sets the error flag */
                    error = 1;

                    /* breaks the switch */
                    break;
            }

            /* breaks the loop */
            break;
        }

        /* prints a debug message */
        V_DEBUG_F("Data [%d bytes] sent without errors\n", numberBytes);

        /* in case the number of bytes sent is the same as the value size
        (not all data has been sent) */
        if(numberBytes != data->size) {
            /* prints a debug message */
            V_DEBUG_F("Shrinking data [%d bytes] (partial message sent)\n", numberBytes);

            /* updates the data internal structure
            to allow the sending of the pending partial data */
            data->data += numberBytes;
            data->size -= numberBytes;

            /* sets the error flag (non fatal) */
            error = 2;

            /* breaks the loop */
            break;
        }

        /* pops a value (data) from the linked list (write queue) */
        popValueLinkedList(connection->writeQueue, (void **) &data, 1);

        /* in case the data callback is set */
        if(data->callback != NULL) {
            /* prints a debug message */
            V_DEBUG("Calling write callback\n");

            /* calls the callback with the callback parameters */
            data->callback(connection, data, data->callbackParameters);

            /* prints a debug message */
            V_DEBUG("Finished calling write callback\n");
        }

        /* prints a debug message */
        V_DEBUG("Deleting data (cleanup structures)\n");

        /* deletes the data */
        deleteData(data);
    }

    /* prints a debug message */
    V_DEBUG_F("Processing write error code: %d\n", error);

    /* switches over the error flag and value */
    switch(error) {
        /* in case there's no error */
        case 0:
            /* unregisters the connection for write */
            connection->unregisterWrite(connection);

            /* breaks the switch */
            break;

        /* in case it's a fatal error */
        case 1:
            /* closes the connection */
            connection->closeConnection(connection);

            /* breaks the switch */
            break;

        /* in case it's a non fatal error */
        case 2:
            /* breaks the switch */
            break;

        /* default case */
        default:
            /* breaks the switch */
            break;
    }

    /* returns the error code */
    return error;
}

ERROR_CODE errorHandlerStreamIo(struct Connection_t *connection) {
    /* closes the connection */
    connection->closeConnection(connection);

    return 0;
}

ERROR_CODE openHandlerStreamIo(struct Connection_t *connection) {
    /* allocates the io connection */
    struct IoConnection_t *ioConnection;

    /* creates the io connection */
    createIoConnection(&ioConnection, connection);

    /* TODO: this values are hardcoded and should be
    removed (change also header file reference) */
    ioConnection->onData = dataHandlerStreamHttp;
    ioConnection->onOpen = openHandlerStreamHttp;
    ioConnection->onClose = closeHandlerStreamHttp;

    /* in case the on open handler is defined */
    if(ioConnection->onOpen != NULL) {
        /* prints a debug message */
        V_DEBUG("Calling on open handler\n");

        /* calls the on open handler */
        ioConnection->onOpen(ioConnection);

        /* prints a debug message */
        V_DEBUG("Finished calling on open handler\n");
    }

    return 0;
}

ERROR_CODE closeHandlerStreamIo(struct Connection_t *connection) {
    /* retrieves the io connection */
    struct IoConnection_t *ioConnection = (struct IoConnection_t *) connection->lower;

    /* in case the on close handler is defined */
    if(ioConnection->onClose != NULL) {
        /* prints a debug message */
        V_DEBUG("Calling on close handler\n");

        /* calls the on close handler */
        ioConnection->onClose(ioConnection);

        /* prints a debug message */
        V_DEBUG("Finished calling on close handler\n");
    }

    /* deletes the io connection */
    deleteIoConnection(ioConnection);

    return 0;
}
