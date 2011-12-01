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

#include "stdafx.h"

#include "type.h"

void createType(struct Type_t **typePointer, enum Type_e _type) {
    /* retrieves the type size */
    size_t typeSize = sizeof(struct Type_t);

    /* allocates space for the type */
    struct Type_t *type = (struct Type_t *) MALLOC(typeSize);

    /* sets the type in the type */
    type->type = _type;

    /* sets the type in the type pointer */
    *typePointer = type;
}

void deleteType(struct Type_t *type) {
    /* releases the type */
    FREE(type);
}

ERROR_CODE toStringType(struct Type_t *type, unsigned char *buffer, size_t bufferSize) {
    /* switches over the type's type in order to
    execute the proper type conversion */
    switch(type->type) {
        case INTEGER_TYPE:
            /* converts the integer value using the
            "default" integer to ascii conversion */
            itoa(type->value.valueInt, buffer, 10);

            /* breaks the switch */
            break;

        case FLOAT_TYPE:
            /* converts the float value using the string
            *conversion function for float values */
            SPRINTF(buffer, bufferSize, "%f", type->value.valueFloat);

            /* breaks the switch */
            break;

        case STRING_TYPE:
            /* copies the memory buffer of the value in string
            representation to the buffer */
            memcpy(buffer, type->value.valueString, strlen(type->value.valueString) + 1);

            /* breaks the switch */
            break;

        default:
            /* copies the undefined value to the buffer (no conversion
            is possible) */
            memcpy(buffer, "undefined", 10);

            /* breaks the switch */
            break;
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

struct Type_t integerType(int value) {
    /* allocates space for the type */
    struct Type_t type;

    /* sets the type's type and value */
    type.type = INTEGER_TYPE;
    type.value.valueInt = value;

    /* returns the type */
    return type;
}

struct Type_t floatType(float value) {
    /* allocates space for the type */
    struct Type_t type;

    /* sets the type's type and value */
    type.type = FLOAT_TYPE;
    type.value.valueFloat = value;

    /* returns the type */
    return type;
}

struct Type_t stringType(char *value) {
    /* allocates space for the type */
    struct Type_t type;

    /* sets the type's type and value */
    type.type = FLOAT_TYPE;
    type.value.valueString = value;

    return type;
}

struct Type_t mapType(struct HashMap *value) {
    /* allocates space for the type */
    struct Type_t type;

    /* sets the type's type and value */
    type.type = MAP_TYPE;
    type.value.valueMap = value;

    /* returns the type */
    return type;
}

struct Type_t listType(struct LinkedList *value) {
    /* allocates space for the type */
    struct Type_t type;

    /* sets the type's type and value */
    type.type = LIST_TYPE;
    type.value.valueList = value;

    /* returns the type */
    return type;
}
