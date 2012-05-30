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

#include "bencoding.h"

void createBencodingEngine(struct BencodingEngine_t **bencodingEnginePointer, void *context) {
    /* retrieves the bencoding engine size */
    size_t bencodingEngineSize = sizeof(struct BencodingEngine_t);

    /* allocates space for the bencoding engine */
    struct BencodingEngine_t *bencodingEngine = (struct BencodingEngine_t *) MALLOC(bencodingEngineSize);

    /* initializes the bencoding engine */
    bencodingEngine->state = BENCODING_ENGINE_NORMAL;
    bencodingEngine->mapType = SORT_MAP_TYPE;
    bencodingEngine->integerEndMark = 0;
    bencodingEngine->stringEndMark = 0;
    bencodingEngine->context = context;

    /* sets the bencoding engine in the bencoding engine pointer */
    *bencodingEnginePointer = bencodingEngine;
}

void deleteBencodingEngine(struct BencodingEngine_t *bencodingEngine) {
    /* releases the bencoding engine */
    FREE(bencodingEngine);
}

void createBencodingHandler(struct BencodingHandler_t **bencodingHandlerPointer) {
    /* retrieves the bencoding handler size */
    size_t bencodingHandlerSize = sizeof(struct BencodingHandler_t);

    /* allocates space for the bencoding handler */
    struct BencodingHandler_t *bencodingHandler = (struct BencodingHandler_t *) MALLOC(bencodingHandlerSize);

    /* initializes the bencoding handler */
    bencodingHandler->sequence = NULL;
    bencodingHandler->top = NULL;
    bencodingHandler->key = NULL;
    bencodingHandler->nextKey = 0;

    /* creates the linked lists for both the sequence and key stack
    values (used for runtime) */
    createLinkedList(&bencodingHandler->sequenceStack);
    createLinkedList(&bencodingHandler->keyStack);

    /* sets the bencoding handler in the bencoding handler pointer */
    *bencodingHandlerPointer = bencodingHandler;
}

void deleteBencodingHandler(struct BencodingHandler_t *bencodingHandler) {
    /* in case the sequence stack and the key stack are defined releases
    their memory to avoid any leaks */
    if(bencodingHandler->sequenceStack) { FREE(bencodingHandler->sequenceStack); }
    if(bencodingHandler->keyStack) { FREE(bencodingHandler->keyStack); }

    /* releases the bencoding handler */
    FREE(bencodingHandler);
}

ERROR_CODE encodeBencoding(struct Type_t *type, unsigned char **encodedBufferPointer, size_t *encodedBufferLengthPointer) {
    /* allocates space for the encoded buffer reference and
    for the string buffer reference */
    unsigned char *encodedBuffer;
    struct StringBuffer_t *stringBuffer;

    /* creates the string buffer an then uses it to support
    the encoding of the provided type (top level type) then
    after the encoding is complete joins the string buffer and
    deletes it from memory*/
    createStringBuffer(&stringBuffer);
    _encodeType(type, stringBuffer);
    joinStringBuffer(stringBuffer, &encodedBuffer);
    deleteStringBuffer(stringBuffer);

    /* updates the references to the encoded buffer pointer and
    the reference to the encoded buffer length (from string length) */
    *encodedBufferPointer = encodedBuffer;
    *encodedBufferLengthPointer = strlen((char *) encodedBuffer);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE decodeBencoding(unsigned char *encodedBuffer, size_t encodedBufferLength, struct Type_t **typePointer) {
    /* allocates space for the general (temporary) variables
    to be used durring the parsing of the buffer */
    struct BencodingEngine_t *bencodingEngine;
    struct BencodingHandler_t *bencodingHandler;

    /* starts the bencoding engine, starting all of its internal
    structures and then runs an engine tick parsing the complete
    file buffer (it's completely available) */
    _startBencodingEngine(&bencodingEngine);
    _runBencodingEngine(bencodingEngine, encodedBuffer, encodedBufferLength);

    /* retieves the handler associated with the engine and then
    uses it to retrieve the top type of the parsed structure */
    bencodingHandler = (struct BencodingHandler_t *) bencodingEngine->context;
    *typePointer = bencodingHandler->top;

    /* stops the bencoding engine releasing all of its internal
    structures, the top type is still defined */
    _stopBencodingEngine(bencodingEngine);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE encodeBencodingFile(char *filePath, struct Type_t *type) {
    /* allocates space for the reference to the buffer
    to hold the encoded contents and for the integer to
    hold the size of that buffer */
    unsigned char *encodedBuffer;
    size_t encodedBufferLength;

    /* runs the encoding process over the provided type structure
    and then uses the resulting buffer to write it to the file */
    encodeBencoding(type, &encodedBuffer, &encodedBufferLength);
    writeFile(filePath, encodedBuffer, encodedBufferLength);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE decodeBencodingFile(char *filePath, struct Type_t **typePointer) {
    /* allocates space for the general (temporary) variables
    to be used durring the parsing of the file */
    ERROR_CODE returnValue;
    size_t fileSize;
    unsigned char *fileBuffer;
    struct BencodingEngine_t *bencodingEngine;
    struct BencodingHandler_t *bencodingHandler;

    /* reads the file contained in the provided file path
    and then tests the error code for error, in case there is an
    error prints it to the error stream output */
    returnValue = readFile(filePath, &fileBuffer, &fileSize);
    if(IS_ERROR_CODE(returnValue)) { RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem reading file"); }

    /* starts the bencoding engine, starting all of its internal
    structures and then runs an engine tick parsing the complete
    file buffer (it's completely available) */
    _startBencodingEngine(&bencodingEngine);
    _runBencodingEngine(bencodingEngine, fileBuffer, fileSize);

    /* retieves the handler associated with the engine and then
    uses it to retrieve the top type of the parsed structure */
    bencodingHandler = (struct BencodingHandler_t *) bencodingEngine->context;
    *typePointer = bencodingHandler->top;

    /* stops the bencoding engine releasing all of its internal
    structures, the top type is still defined */
    _stopBencodingEngine(bencodingEngine);

    /* releases the buffer used durring the parsing of
    the configuration file */
    FREE(fileBuffer);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _encodeType(struct Type_t *type, struct StringBuffer_t *stringBuffer) {
    char *buffer;
    struct Type_t key;
    struct Type_t *current;
    struct Iterator_t *iterator;
    struct HashMapElement_t *element;

    switch(type->type) {
        case INTEGER_TYPE:
            buffer = MALLOC(16);
            SPRINTF(buffer, 16, "%d", type->value.valueInt);

            appendStringBuffer(stringBuffer, (unsigned char *) "i");
            _appendStringBuffer(stringBuffer, (unsigned char *) buffer);
            appendStringBuffer(stringBuffer, (unsigned char *) "e");

            break;

        case STRING_TYPE:
            buffer = MALLOC(16);
            SPRINTF(buffer, 16, "%lu", strlen(type->value.valueString));

            _appendStringBuffer(stringBuffer, (unsigned char *) buffer);
            appendStringBuffer(stringBuffer, (unsigned char *) ":");
            appendStringBuffer(stringBuffer, (unsigned char *) type->value.valueString);

            break;

        case LIST_TYPE:
            appendStringBuffer(stringBuffer, (unsigned char *) "l");

            createIteratorLinkedList(type->value.valueList, &iterator);

            while(1) {
                getNextIterator(iterator, (void **) &current);
                if(current == NULL) { break; }
                _encodeType(current, stringBuffer);
            }

            deleteIteratorLinkedList(type->value.valueList, iterator);

            appendStringBuffer(stringBuffer, (unsigned char *) "e");

            break;

        case MAP_TYPE:
            appendStringBuffer(stringBuffer, (unsigned char *) "d");

            createElementIteratorHashMap(type->value.valueMap, &iterator);

            while(1) {
                getNextIterator(iterator, (void **) &element);
                if(element == NULL) { break; }
                key = stringType((char *) element->keyString);
                _encodeType(&key, stringBuffer);
                _encodeType((struct Type_t *) element->value, stringBuffer);
            }

            deleteIteratorHashMap(type->value.valueMap, iterator);

            appendStringBuffer(stringBuffer, (unsigned char *) "e");

            break;

        case SORT_MAP_TYPE:
            appendStringBuffer(stringBuffer, (unsigned char *) "d");

            createElementIteratorSortMap(type->value.valueSortMap, &iterator);

            while(1) {
                getNextIterator(iterator, (void **) &element);
                if(element == NULL) { break; }
                key = stringType((char *) element->keyString);
                _encodeType(&key, stringBuffer);
                _encodeType((struct Type_t *) element->value, stringBuffer);
            }

            deleteIteratorSortMap(type->value.valueSortMap, iterator);

            appendStringBuffer(stringBuffer, (unsigned char *) "e");

            break;

        default:
            break;
    }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _startBencodingEngine(struct BencodingEngine_t **bencodingEnginePointer) {
    /* allocates space for the various bencoding
    related structures for usage in runtime */
    struct BencodingEngine_t *bencodingEngine;
    struct BencodingHandler_t *bencodingHandler;
    struct BencodingSettings_t *bencodingSettings;

    /* creates the bencoding handler structure and then creates
    the bencoding engine setting the handler as the context */
    createBencodingHandler(&bencodingHandler);
    createBencodingEngine(&bencodingEngine, (void *) bencodingHandler);

    /* retrieves the memory reference to the engine settings */
    bencodingSettings = &bencodingEngine->settings;

    /* sets the various handlers for the bencoding settings
    parsing, they will be used to correctly update the
    provided configuration hash map */
    bencodingSettings->onintegerStart = NULL;
    bencodingSettings->onintegerEnd = _bencodingIntegerEndCallback;
    bencodingSettings->onstringStart = NULL;
    bencodingSettings->onstringEnd = _bencodingStringEndCallback;
    bencodingSettings->onlistStart = _bencodingListStartCallback;
    bencodingSettings->ondictionaryStart = _bencodingDictionaryStartCallback;
    bencodingSettings->onsequenceEnd = _bencodingSequenceEndCallback;

    /* sets the bencoding engine in the bencoding engine pointer,
    this allows the caller function to use the engine */
    *bencodingEnginePointer = bencodingEngine;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _stopBencodingEngine(struct BencodingEngine_t *bencodingEngine) {
    /* deletes both the handler referenced as context in the engine and the
    current context engine (memory cleanup) */
    deleteBencodingHandler((struct BencodingHandler_t *) bencodingEngine->context);
    deleteBencodingEngine(bencodingEngine);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _runBencodingEngine(struct BencodingEngine_t *bencodingEngine, unsigned char *buffer, size_t size) {
    /* allocates space for the general (temporary) variables
    to be used durring the parsing of the buffer */
    size_t index;
    size_t stringSize;
    unsigned char character;
    enum BencodingState_e state;

    /* allocates the mark variables used to locate
    the part of context changing durring the parsing */
    unsigned char *pointer = 0;
    unsigned char *integerEndMark = 0;
    unsigned char *stringEndMark = 0;

    /* updates the current state value from the engine
    references (state update) */
    state = bencodingEngine->state;
    integerEndMark = bencodingEngine->integerEndMark;
    stringEndMark = bencodingEngine->stringEndMark;

    /* initializes the string size value, avoids
    possible problems with no variable initialization */
    stringSize = 0;

    /* iterates over the byte range of the file, all the bytes
    should be contained in the buffer "under" iteration */
    for(index = 0; index < size; index++) {
        /* retrieves the current character from the
        file buffer and the retrieves the pointer to
        its position */
        character = buffer[index];
        pointer = &buffer[index];

        switch(state) {
            case BENCODING_ENGINE_NORMAL:
                switch(character) {
                    case 'i':
                        state = BENCODING_ENGINE_INTEGER;

                        BENCODING_MARK_N(integerEnd, -1);
                        BENCODING_CALLBACK(integerStart);

                        break;

                    case 'l':
                        BENCODING_CALLBACK(listStart);

                        break;

                    case 'd':
                        BENCODING_CALLBACK(dictionaryStart);

                        break;

                    case 'e':
                        BENCODING_CALLBACK(sequenceEnd);

                        break;

                    case ' ':
                        break;

                    default:
                        state = BENCODING_ENGINE_STRING_SIZE;
                        stringSize = character - '0';

                        break;
                }

                break;

            case BENCODING_ENGINE_INTEGER:
                if(character == 'e') {
                    state = BENCODING_ENGINE_NORMAL;

                    BENCODING_CALLBACK_DATA(integerEnd);
                }

                break;

            case BENCODING_ENGINE_STRING:
                if(stringSize == 1) {
                    state = BENCODING_ENGINE_NORMAL;

                    BENCODING_CALLBACK_DATA_N(stringEnd, -1);
                } else {
                    stringSize--;
                }

                break;

            case BENCODING_ENGINE_STRING_SIZE:
                if(character == ':') {
                    state = BENCODING_ENGINE_STRING;

                    BENCODING_MARK_N(stringEnd, -1);
                    BENCODING_CALLBACK(stringStart);
                } else {
                    stringSize *= 10;
                    stringSize += character - '0';
                }

                break;

            default:
                break;
        }
    }

    /* saves the various state related values back
    to the bencoding engine */
    bencodingEngine->state = state;
    bencodingEngine->integerEndMark = integerEndMark;
    bencodingEngine->stringEndMark = stringEndMark;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _bencodingIntegerEndCallback(struct BencodingEngine_t *bencodingEngine, const unsigned char *pointer, size_t size) {
    /* allocates space for the integer value and for
    the type that will encapsulate it */
    int _integer;
    struct Type_t *type;

    /* retrieves the bencoding handler from the template engine context
    then uses it to store the (current) value */
    struct BencodingHandler_t *bencodingHandler = (struct BencodingHandler_t *) bencodingEngine->context;

    /* allocates space for the integer string value and then copies
    the data buffer into it and terminates the null string */
    char *integer = MALLOC(size + 1);
    memcpy(integer, pointer, size);
    integer[size] = '\0';

    if(bencodingHandler->sequence != NULL) {
        /* converts the integer string into an integer value, then creates
        the type structure for the integer and sets it as an integer value */
        _integer = atoi(integer);
        createType(&type, INTEGER_TYPE);
        *type = integerType(_integer);

        /* switches over the type of current sequence to
        execute the proper operations */
        switch(bencodingHandler->sequence->type) {
            case LIST_TYPE:
                /* adds the current type to the list sequence */
                appendValueLinkedList(bencodingHandler->sequence->value.valueList, (void *) type);

                /* breaks the switch */
                break;

            case MAP_TYPE:
                /* sets the value in the map for the current key and sets the next key
                flag so that the next string is saved as a key */
                setValueStringHashMap(bencodingHandler->sequence->value.valueMap, bencodingHandler->key, (void *) type);
                bencodingHandler->nextKey = 1;

                /* breaks the switch */
                break;

            case SORT_MAP_TYPE:
                /* sets the value in the map for the current key and sets the next key
                flag so that the next string is saved as a key */
                setValueStringSortMap(bencodingHandler->sequence->value.valueSortMap, bencodingHandler->key, (void *) type);
                bencodingHandler->nextKey = 1;

                /* breaks the switch */
                break;

            default:
                /* breaks the switch */
                break;
        }
    }

    /* in case there is no top type defined for the handler sets the
    current type as the top type (base type) */
    if(bencodingHandler->top == NULL) { bencodingHandler->top = type; }

    /* releases the memory associated with the intger string value
    in order to avoid memory leaks */
    FREE(integer);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _bencodingStringEndCallback(struct BencodingEngine_t *bencodingEngine, const unsigned char *pointer, size_t size) {
    /* allocates space for the reference to the
    type structure to hold the string */
    struct Type_t *type;

    /* retrieves the bencoding handler from the template engine context
    then uses it to store the (current) value */
    struct BencodingHandler_t *bencodingHandler = (struct BencodingHandler_t *) bencodingEngine->context;

    /* allocates memory for the string and then copies the source pointer
    data into the string buffer for the provided size and then closes it */
    char *string = MALLOC(size + 1);
    memcpy(string, pointer, size);
    string[size] = '\0';

    /* in case the next key flag is active must set the current key as
    the just retrieved string */
    if(bencodingHandler->nextKey == 1) {
        /* in case there's a key pending to be release must release it
        to avoid memory leaks, then sets the current string as the key
        and unsets the next key flag to save the value */
        if(bencodingHandler->key != NULL) { FREE(bencodingHandler->key); }
        bencodingHandler->key = (unsigned char *) string;
        bencodingHandler->nextKey = 0;
    }
    /* otherwise in case the sequence type is defined the string must
    be a value and must be associated with the sequence */
    else if(bencodingHandler->sequence != NULL) {
        /* creates a new type structure for the string
        and sets it with the correct string value */
        createType(&type, STRING_TYPE);
        *type = stringType(string);

        /* switches over the type of current sequence to
        execute the proper operations */
        switch(bencodingHandler->sequence->type) {
            case LIST_TYPE:
                /* adds the current type to the list sequence */
                appendValueLinkedList(bencodingHandler->sequence->value.valueList, (void *) type);

                /* breaks the switch */
                break;

            case MAP_TYPE:
                /* sets the value in the map for the current key and sets the next key
                flag so that the next string is saved as a key */
                setValueStringHashMap(bencodingHandler->sequence->value.valueMap, bencodingHandler->key, (void *) type);
                bencodingHandler->nextKey = 1;

                /* breaks the switch */
                break;

            case SORT_MAP_TYPE:
                /* sets the value in the map for the current key and sets the next key
                flag so that the next string is saved as a key */
                setValueStringSortMap(bencodingHandler->sequence->value.valueSortMap, bencodingHandler->key, (void *) type);
                bencodingHandler->nextKey = 1;

                /* breaks the switch */
                break;

            default:
                /* breaks the switch */
                break;
        }
    }

    /* in case there is no top type defined for the handler sets the
    current type as the top type (base type) */
    if(bencodingHandler->top == NULL) { bencodingHandler->top = type; }

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _bencodingListStartCallback(struct BencodingEngine_t *bencodingEngine) {
    /* allocates space for the linked list and for the type
    structure that will encapsulate it */
    struct LinkedList_t *list;
    struct Type_t *type;

    /* retrieves the bencoding handler from the template engine context
    then uses it to store the (current) value */
    struct BencodingHandler_t *bencodingHandler = (struct BencodingHandler_t *) bencodingEngine->context;

    /* creates a new linked list for the new structure (sequence) context
    and then creates the respective type for the list */
    createLinkedList(&list);
    createType(&type, LIST_TYPE);
    *type = listType(list);

    /* adds the sequence type to the sequence stack and then adds the
    current handler key to the key stack, then updates the current sequence
    reference and the current key in the handler */
    appendValueLinkedList(bencodingHandler->sequenceStack, (void *) type);
    appendValueLinkedList(bencodingHandler->keyStack, (void *) bencodingHandler->key);
    bencodingHandler->sequence = type;
    bencodingHandler->key = NULL;

    /* in case there is no top type defined for the handler sets the
    current type as the top type (base type) */
    if(bencodingHandler->top == NULL) { bencodingHandler->top = type; }

    /* unsets the next key flag to avoid any possible misbehavior, colliding
    with the hash map structure */
    bencodingHandler->nextKey = 0;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _bencodingDictionaryStartCallback(struct BencodingEngine_t *bencodingEngine) {
    /* allocates space for the hash map and for the type
    structure that will encapsulate it */
    struct HashMap_t *hashMap;
    struct SortMap_t *sortMap;
    struct Type_t *type;

    /* retrieves the bencoding handler from the template engine context
    then uses it to store the (current) value */
    struct BencodingHandler_t *bencodingHandler = (struct BencodingHandler_t *) bencodingEngine->context;

    /* switchs over the "target" map type to be used to store conceptual
    maps in the bencoding structure */
    switch(bencodingEngine->mapType) {
        case MAP_TYPE:
            /* creates a new hash map for the new structure (sequence) context
            and then creates the respective type for the map */
            createHashMap(&hashMap, 0);
            createType(&type, MAP_TYPE);
            *type = mapType(hashMap);

        case SORT_MAP_TYPE:
            /* creates a new sort map for the new structure (sequence) context
            and then creates the respective type for the map */
            createSortMap(&sortMap, 0);
            createType(&type, SORT_MAP_TYPE);
            *type = sortMapType(sortMap);

        default:
            /* creates a new sort map for the new structure (sequence) context
            and then creates the respective type for the map */
            createSortMap(&sortMap, 0);
            createType(&type, SORT_MAP_TYPE);
            *type = sortMapType(sortMap);
    }

    /* adds the sequence type to the sequence stack and then adds the
    current handler key to the key stack, then updates the current sequence
    reference and the current key in the handler */
    appendValueLinkedList(bencodingHandler->sequenceStack, (void *) type);
    appendValueLinkedList(bencodingHandler->keyStack, (void *) bencodingHandler->key);
    bencodingHandler->sequence = type;
    bencodingHandler->key = NULL;

    /* in case there is no top type defined for the handler sets the
    current type as the top type (base type) */
    if(bencodingHandler->top == NULL) { bencodingHandler->top = type; }

    /* sets the next key flag so that in the next iteration the string
    is "accepted" as the current key value */
    bencodingHandler->nextKey = 1;

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _bencodingSequenceEndCallback(struct BencodingEngine_t *bencodingEngine) {
    /* allocates space for the map key for the sequence
    type and for the current type */
    unsigned char *key;
    struct Type_t *sequence;
    struct Type_t *type;

    /* retrieves the bencoding handler from the template engine context
    then uses it to store the (current) value */
    struct BencodingHandler_t *bencodingHandler = (struct BencodingHandler_t *) bencodingEngine->context;

    /* pops the current key in the stack and the current sequence in the
    stack (these are the current values), then peeks the current sequence
    in the stack (in case it exists) it should represent the sequence to
    be set in use for the current context */
    popTopValueLinkedList(bencodingHandler->keyStack, (void **) &key, 1);
    popTopValueLinkedList(bencodingHandler->sequenceStack, (void **) &sequence, 1);
    peekTopValueLinkedList(bencodingHandler->sequenceStack, (void **) &type);
    bencodingHandler->sequence = type;

    /* in case the current (previous) sequence is a map, must do some
    garbage collection to avoid leeks */
    if(sequence->type == MAP_TYPE || sequence->type == SORT_MAP_TYPE) {
        /* in case there is a key currently defined in the handler
        must release its memory (to avoid any leaks) */
        if(bencodingHandler->key != NULL) { FREE(bencodingHandler->key); }
        bencodingHandler->key = NULL;
    }

    /* in case there is no sequence defined for the current handler context
    no need to continue the processing (nothing to be assiciated) this is
    typical for the top level of parsing */
    if(bencodingHandler->sequence == NULL) { RAISE_NO_ERROR; }

    /* swithces over the sequence type to take the appropriate action of
    setting the lower (previous) context in the upper (current) context */
    switch(bencodingHandler->sequence->type) {
        case LIST_TYPE:
            /* adds the lower value to the upper list appending it to the
            back of the lis */
            appendValueLinkedList(bencodingHandler->sequence->value.valueList, (void *) sequence);

            /* unsets the next key flag to avoid any unexpected string parsing
            behavior (hash map only) */
            bencodingHandler->nextKey = 0;

            /* breaks the switch */
            break;

        case MAP_TYPE:
            /* sets the lower value in the upper map for the current key
            this is the lower upper layer association */
            setValueStringHashMap(bencodingHandler->sequence->value.valueMap, key, (void *) sequence);

            /* sets the retrieved key as the current key and
            sets the next key flag to force the retrieval of key */
            bencodingHandler->key = key;
            bencodingHandler->nextKey = 1;

            /* breaks the switch */
            break;

        case SORT_MAP_TYPE:
            /* sets the lower value in the upper map for the current key
            this is the lower upper layer association */
            setValueStringSortMap(bencodingHandler->sequence->value.valueSortMap, key, (void *) sequence);

            /* sets the retrieved key as the current key and
            sets the next key flag to force the retrieval of key */
            bencodingHandler->key = key;
            bencodingHandler->nextKey = 1;

            /* breaks the switch */
            break;

        default:
            /* breaks the switch */
            break;
    }

    /* raises no error */
    RAISE_NO_ERROR;
}
