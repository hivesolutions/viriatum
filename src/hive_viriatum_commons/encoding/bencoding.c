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

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// ERROR_CODE freeType() {} // LIBERTA A MEMORIA RECURSIVAMENTE !!!!
// TENHO MESMO DE IMPLEMENTAR ISTO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

ERROR_CODE printType(struct Type_t *type) {
	struct Type_t *current;
	struct Type_t key;
	struct Iterator_t *iterator;
	struct HashMapElement_t *element;
	unsigned char isFirst = 1;

	switch(type->type) {
        case INTEGER_TYPE:
            PRINTF_F("%d", type->value.valueInt);

            /* breaks the switch */
            break;

        case FLOAT_TYPE:
            PRINTF_F("%f", type->value.valueFloat);

            /* breaks the switch */
            break;

        case STRING_TYPE:
            PRINTF_F("'%s'", type->value.valueString);

            /* breaks the switch */
            break;

        case LIST_TYPE:
            PRINTF("[");

			createIteratorLinkedList(type->value.valueList, &iterator);

			while(1) {
				getNextIterator(iterator, (void **) &current);
				if(current == NULL) { break; }
				if(isFirst == 0) { PRINTF(", "); };
				printType(current);
				isFirst = 0;
			}

			deleteIteratorLinkedList(type->value.valueList, iterator);

			PRINTF("]");

            /* breaks the switch */
            break;

		case MAP_TYPE:
			PRINTF("{");

			createElementIteratorHashMap(type->value.valueMap, &iterator);

			while(1) {
				getNextIterator(iterator, (void **) &element);
				if(element == NULL) { break; }
				if(isFirst == 0) { PRINTF(", "); };
				key = stringType(element->keyString);
				printType(&key);
				PRINTF(" : ");
				printType((struct Type_t *) element->value);
				isFirst = 0;
			}

			deleteIteratorHashMap(type->value.valueMap, iterator);

			PRINTF("}");

            /* breaks the switch */
            break;

		default:
            PRINTF("undefined");

			/* breaks the switch */
			break;
	}

    /* raises no error */
    RAISE_NO_ERROR;
}


ERROR_CODE processBencodingFile(char *filePath, struct Type_t **typePointer) {
	size_t stringSize;



    /* allocates space for the general (temporary) variables
    to be used durring the parsing of the file */
    ERROR_CODE returnValue;
    size_t index;
    size_t fileSize;
    unsigned char *fileBuffer;
    unsigned char character;
    enum BencodingState_e state;

    /* allocates the mark variables used to locate
    the part of context changing durring the parsing */
    unsigned char *pointer = 0;
    unsigned char *integerEndMark = 0;
    unsigned char *stringEndMark = 0;

    /* allocates space for the settings to be used by
    the engine for the engine instance itself and for
    the handler to be used to "catch" the events, then
    retrieves the pointers to these structures*/
    struct BencodingSettings_t bencodingSettings_s;
    struct BencodingEngine_t bencodingEngine_s;
    struct BencodingHandler_t bencodingHandler_s;
    struct BencodingSettings_t *bencodingSettings = &bencodingSettings_s;
    struct BencodingEngine_t *bencodingEngine = &bencodingEngine_s;
    struct BencodingHandler_t *bencodingHandler = &bencodingHandler_s;

    /* allocates space for the sequence and key stacks to be used
    during the runtime of the parser */
    struct LinkedList_t *sequenceStack;
	struct LinkedList_t *keyStack;

    createLinkedList(&sequenceStack);
	createLinkedList(&keyStack);




    /* sets the various handlers for the bencoding settings
    parsing, they will be used to correctly update the
    provided configuration hash map */
    bencodingSettings_s.onintegerStart = NULL;
    bencodingSettings_s.onintegerEnd = _bencodingIntegerEndCallback;
    bencodingSettings_s.onstringStart = NULL;
    bencodingSettings_s.onstringEnd = _bencodingStringEndCallback;
    bencodingSettings_s.onlistStart = _bencodingListStartCallback;
    bencodingSettings_s.ondictionaryStart = _bencodingDictionaryStartCallback;
    bencodingSettings_s.onsequenceEnd = _bencodingSequenceEndCallback;

    /* sets the configuration reference in the bencoding handler
    so that it may be updatd and then sets the handler in
    the bencoding engine instance to be used for parsing */
   


	bencodingHandler_s.sequenceStack = sequenceStack;
	bencodingHandler_s.keyStack = keyStack;
	bencodingHandler_s.sequence = NULL;
	bencodingHandler_s.top = NULL;
	bencodingHandler_s.key = NULL;
	bencodingHandler_s.nextKey = 0;
    bencodingEngine_s.context = bencodingHandler;





    /* reads the file contained in the provided file path
    and then tests the error code for error, in case there is an
    error prints it to the error stream output */
    returnValue = readFile(filePath, &fileBuffer, &fileSize);
    if(IS_ERROR_CODE(returnValue)) { RAISE_ERROR_M(RUNTIME_EXCEPTION_ERROR_CODE, (unsigned char *) "Problem reading file"); }

    /* sets the initial state for the parsing process this
    is considered to be the "general loop" state */
    state = BENCODING_ENGINE_NORMAL;

    /* iterates over the byte range of the file, all the bytes
    should be contained in the buffer "under" iteration */
    for(index = 0; index < fileSize; index++) {
        /* retrieves the current character from the
        file buffer and the retrieves the pointer to
        its position */
        character = fileBuffer[index];
        pointer = &fileBuffer[index];

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
        }
    }



	printType(bencodingHandler_s.top);




    /* releases the buffer used durring the parsing of
    the configuration file */
    FREE(fileBuffer);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _bencodingIntegerEndCallback(struct BencodingEngine_t *bencodingEngine, const unsigned char *pointer, size_t size) {
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

	/* converts the integer string into an integer value, then creates
	the type structure for the integer and sets it as an integer value */
	_integer = atoi(integer);
	createType(&type, INTEGER_TYPE);
	*type = integerType(_integer);

	if(bencodingHandler->sequence != NULL) { 
		switch(bencodingHandler->sequence->type) {
			case LIST_TYPE:
				appendValueLinkedList(bencodingHandler->sequence->value.valueList, (void *) type);

				break;

			case MAP_TYPE:
				setValueStringHashMap(bencodingHandler->sequence->value.valueMap, bencodingHandler->key, (void *) type);
				bencodingHandler->nextKey = 1;

				break;
		}
	}
	if(bencodingHandler->top == NULL) { bencodingHandler->top = type; }

	/* releases the memory associated with the intger string value
	in order to avoid memory leaks */
	FREE(integer);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _bencodingStringEndCallback(struct BencodingEngine_t *bencodingEngine, const unsigned char *pointer, size_t size) {
	struct Type_t *type;

    /* retrieves the bencoding handler from the template engine context
    then uses it to store the (current) value */
    struct BencodingHandler_t *bencodingHandler = (struct BencodingHandler_t *) bencodingEngine->context;

    char *string = MALLOC(size + 1);

	memcpy(string, pointer, size);
	string[size] = '\0';

	createType(&type, STRING_TYPE);
	*type = stringType(string);

	if(bencodingHandler->nextKey == 1) {
		bencodingHandler->key = string;
		bencodingHandler->nextKey = 0;
	} else if(bencodingHandler->sequence != NULL) { 
		switch(bencodingHandler->sequence->type) {
			case LIST_TYPE:
				appendValueLinkedList(bencodingHandler->sequence->value.valueList, (void *) type);

				break;

			case MAP_TYPE:
				setValueStringHashMap(bencodingHandler->sequence->value.valueMap, bencodingHandler->key, (void *) type);
				bencodingHandler->nextKey = 1;

				break;
		}
	}
	if(bencodingHandler->top == NULL) { bencodingHandler->top = type; }

	/* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _bencodingListStartCallback(struct BencodingEngine_t *bencodingEngine) {
	struct LinkedList_t *list;
	struct Type_t *type;

    /* retrieves the bencoding handler from the template engine context
    then uses it to store the (current) value */
    struct BencodingHandler_t *bencodingHandler = (struct BencodingHandler_t *) bencodingEngine->context;

	createType(&type, LIST_TYPE);

	createLinkedList(&list);
	*type = listType(list);

	bencodingHandler->sequence = type;
	if(bencodingHandler->top == NULL) { bencodingHandler->top = type; }

	appendValueLinkedList(bencodingHandler->sequenceStack, (void *) type);
	appendValueLinkedList(bencodingHandler->keyStack, (void *) NULL);

	/* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _bencodingDictionaryStartCallback(struct BencodingEngine_t *bencodingEngine) {
	struct HashMap_t *hashMap;
	struct Type_t *type;

    /* retrieves the bencoding handler from the template engine context
    then uses it to store the (current) value */
    struct BencodingHandler_t *bencodingHandler = (struct BencodingHandler_t *) bencodingEngine->context;

	createType(&type, MAP_TYPE);

	createHashMap(&hashMap, 0);
	*type = mapType(hashMap);

	bencodingHandler->sequence = type;
	if(bencodingHandler->top == NULL) { bencodingHandler->top = type; }
	appendValueLinkedList(bencodingHandler->sequenceStack, (void *) type);
	appendValueLinkedList(bencodingHandler->keyStack, (void *) bencodingHandler->key);

	/* sets the next key flag so that in the next iteration the string
	is "accepted" as the current key value */
	bencodingHandler->nextKey = 1;

	/* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _bencodingSequenceEndCallback(struct BencodingEngine_t *bencodingEngine) {
	char *key;
	struct Type_t *sequence;
	struct Type_t *type;

    /* retrieves the bencoding handler from the template engine context
    then uses it to store the (current) value */
    struct BencodingHandler_t *bencodingHandler = (struct BencodingHandler_t *) bencodingEngine->context;

	popTopValueLinkedList(bencodingHandler->keyStack, (void **) &key, 1);

	popTopValueLinkedList(bencodingHandler->sequenceStack, (void **) &sequence, 1);
	peekTopValueLinkedList(bencodingHandler->sequenceStack, (void **) &type);
	bencodingHandler->sequence = type;

	if(bencodingHandler->sequence != NULL) { 
		switch(bencodingHandler->sequence->type) {
			case LIST_TYPE:
				appendValueLinkedList(bencodingHandler->sequence->value.valueList, (void *) sequence);

				break;

			case MAP_TYPE:
				setValueStringHashMap(bencodingHandler->sequence->value.valueMap, key, (void *) sequence);

				break;
		}
	}

	bencodingHandler->nextKey = 1;

	/* raises no error */
    RAISE_NO_ERROR;
}

int encodeBencoding(unsigned char *buffer, size_t bufferLength, unsigned char **encodedBufferPointer, size_t *encodedBufferLengthPointer) {
    /* returns valid */
    return 0;
}

int decodeBencoding(unsigned char *encodedBuffer, size_t encodedBufferLength, unsigned char **decodedBufferPointer, size_t *decodedBufferLengthPointer) {
    /* returns valid */
    return 0;
}
