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

    /* allocates space for the hash map to be used for
    the configuration to be created */
   /* struct HashMap_t *configuration;*/



    /* creates the hash map that will hold the various
    arguments, then updates the configuration pointer
    with the created configuration hash map */
   /* createHashMap(&configuration, 0);
    *typePointer = configuration;*/




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
   
	/* bencodingHandler_s.configuration = configuration; */
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

    /* releases the buffer used durring the parsing of
    the configuration file */
    FREE(fileBuffer);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _bencodingIntegerEndCallback(struct BencodingEngine_t *bencodingEngine, const unsigned char *pointer, size_t size) {
    char *integer = malloc(size + 1);

	memcpy(integer, pointer, size);
	integer[size] = '\0';

	printf("INTEGER -> %s\n", integer);

	free(integer);

    /* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _bencodingStringEndCallback(struct BencodingEngine_t *bencodingEngine, const unsigned char *pointer, size_t size) {
    char *string = malloc(size + 1);

	memcpy(string, pointer, size);
	string[size] = '\0';

	printf("STRING -> %s\n", string);

	free(string);

	/* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _bencodingListStartCallback(struct BencodingEngine_t *bencodingEngine) {
	printf("NEW LIST\n");

	/* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _bencodingDictionaryStartCallback(struct BencodingEngine_t *bencodingEngine) {
	printf("NEW Dictionary\n");

	/* raises no error */
    RAISE_NO_ERROR;
}

ERROR_CODE _bencodingSequenceEndCallback(struct BencodingEngine_t *bencodingEngine) {
	printf("END SEQUENCE\n");

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
