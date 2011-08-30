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

ERROR_CODE readHandlerStreamIo(struct Connection_t *connection) {
    /* allocates the number of bytes */
    size_t numberBytes;

    /* allocates the "simple" buffer */
    unsigned char buffer[10240];

    /* retrieves the buffer pointer */
    unsigned char *bufferPointer = (unsigned char *) buffer;

    /* allocates the buffer size */
    size_t bufferSize = 0;

    /* flag and value controlling the state of the read */
    ERROR_CODE error = 0;

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
            /* calls the on data handler */
            dataHandlerStreamHttp(connection, buffer, bufferSize);

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
            /* calls the on data handler */
            dataHandlerStreamHttp(connection, buffer, bufferSize);

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
    int numberBytes;

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
            /* updates the data internal structure
            to allow the sending of the pending partial data */
            data->data += numberBytes;
            data->size += data->size - numberBytes;

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

        /* deletes the data */
        deleteData(data);
    }

    /* switches over the error flag and value */
    switch(error) {
        /* in case there's no error */
        case 0:
            /* unregisters the connection for write */
            connection->service->unregisterWrite(connection);

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
