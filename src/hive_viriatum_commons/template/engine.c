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

#include "engine.h"

void createTemplateEngine(struct TemplateEngine_t **templateEnginePointer) {
    /* retrieves the template engine size */
    size_t templateEngineSize = sizeof(struct TemplateEngine_t);

    /* allocates space for the template engine */
    struct TemplateEngine_t *templateEngine = (struct TemplateEngine_t *) MALLOC(templateEngineSize);

    /* sets the template engine in the template engine pointer */
    *templateEnginePointer = templateEngine;
}

void deleteTemplateEngine(struct TemplateEngine_t *templateEngine) {
    /* releases the template engine */
    FREE(templateEngine);
}

void processTemplateEngine(struct TemplateEngine_t *templateEngine, unsigned char *filePath) {
    /* allocates space for the file */
    FILE *file;

    /* allocats space for the current (character) for reading
    and for the ahead character for look ahead*/
    char current;
    char ahead;

    /* allocates the space for the look ahead valid flag */
    unsigned char aheadSet = 0;

    size_t fileSize;

    unsigned char *fileBuffer;

    /* allocates the index variable used to locate
    the part of context changing durring the parsing */
    size_t index = 0;
    size_t tagIndex = 0;
    size_t parameterIndex = 0;
    size_t parameterValueIndex = 0;

    /* allocates and starts the state for the template parsing */
    enum TemplateEngineState_e state = TEMPLATE_ENGINE_NORMAL;

    unsigned char buffer[4096];

    /* opens the file */
    FOPEN(&file, (char *) filePath, "rb");

    /* retrieves the size of the file by seeking to the
    end of it and the seeks the stream back to the initial
    position (for further reading) */
    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    /* allocates the buffer that will hold the complete
    template file (this allocation may be giant), this is
    necessary for the correct execution of the parser */
    fileBuffer = (void *) MALLOC(fileSize);

    /* iterates continuously too run the parser
    over the complete set of file contents */
    while(1) {
        /* in case the look ahead mode is set, should
        read from the look ahead instead of the normal
        file reading */
        if(aheadSet) {
            /* sets the current read character as the look
            ahead character */
            current = ahead;
        }
        /* otherwise it must be a normal reading */
        else {
            /* retrieves the current character
            from the file stream */
            current = _getcTemplateEngine(file, fileBuffer, &index);
        }

        /* in case the end of file has been found */
        if(current == EOF) {
            /* breaks the cycle (end of parsing) */
            break;
        }

        /* switches over the state to determine the appropriate
        handling to be made for the current character */
        switch(state) {
            case TEMPLATE_ENGINE_NORMAL:
                if(current == '$') {
                    state = TEMPLATE_ENGINE_DOLAR;
                }

                break;

            case TEMPLATE_ENGINE_DOLAR:
                if(current == '{') {
                    state = TEMPLATE_ENGINE_OPEN;

                    /* RAISE tag open index - 1 */
                    tagIndex = index + 1;
                } else {
                    state = TEMPLATE_ENGINE_NORMAL;
                }

                break;

            case TEMPLATE_ENGINE_OPEN:
                if(current == '/') {
                    ahead = _getcTemplateEngine(file, fileBuffer, &index);
                    aheadSet = 1;

                    if(ahead == '}') {
                        state = TEMPLATE_ENGINE_NORMAL;
                        aheadSet = 0;

                        printf("\n");

                        break;
                    }
                }

                if(current == ' ') {
                    /* RAISE new tag[tagIndex:index]  */
                    memcpy(buffer, fileBuffer + tagIndex, index - tagIndex);
                    buffer[index - tagIndex] = '\0';

                    printf("NEW TAG: %s\n", buffer);

                    state = TEMPLATE_ENGINE_PARAMETERS;
                }

                break;

            case TEMPLATE_ENGINE_PARAMETERS:
                if(current == '/') {
                    ahead = _getcTemplateEngine(file, fileBuffer, &index);
                    aheadSet = 1;

                    if(ahead == '}') {
                        state = TEMPLATE_ENGINE_NORMAL;
                        aheadSet = 0;

                        break;
                    }
                }

                if(current != ' ') {
                    state = TEMPLATE_ENGINE_PARAMETER;

                    parameterIndex = index;
                }

                break;

            case TEMPLATE_ENGINE_PARAMETER:
                if(current == '/') {
                    ahead = _getcTemplateEngine(file, fileBuffer, &index);
                    aheadSet = 1;

                    if(ahead == '}') {
                        state = TEMPLATE_ENGINE_NORMAL;
                        aheadSet = 0;

                        break;
                    }
                }

                if(current == '=') {
                    state = TEMPLATE_ENGINE_PARAMETER_VALUE;

                    memcpy(buffer, fileBuffer + parameterIndex, index - parameterIndex);
                    buffer[index - parameterIndex] = '\0';

                    printf("NEW PARAMETER: %s\n", buffer);

                    /* RAISE new parameter[parameterIndex:index] */

                    parameterValueIndex = index + 1;
                }

                break;

            case TEMPLATE_ENGINE_PARAMETER_VALUE:
                if(current == '/') {
                    ahead = _getcTemplateEngine(file, fileBuffer, &index);
                    aheadSet = 1;

                    if(ahead == '}') {
                        state = TEMPLATE_ENGINE_NORMAL;
                        aheadSet = 0;

                        printf("\n");

                        break;
                    }
                }

                if(current == '\"') {
                    state = TEMPLATE_ENGINE_PARAMETER_VALUE_STRING;
                } else if(current == ' ') {
                    memcpy(buffer, fileBuffer + parameterValueIndex, index - parameterValueIndex);
                    buffer[index - parameterValueIndex] = '\0';

                    printf("NEW PARAMETER VALUE: %s\n", buffer);

                    state = TEMPLATE_ENGINE_PARAMETERS;
                }

                break;

            case TEMPLATE_ENGINE_PARAMETER_VALUE_STRING:
                if(current == '\"') {
                    memcpy(buffer, fileBuffer + parameterValueIndex, index - parameterValueIndex + 1);
                    buffer[index - parameterValueIndex + 1] = '\0';

                    printf("NEW (STRING) PARAMETER VALUE: %s\n", buffer);

                    state = TEMPLATE_ENGINE_PARAMETERS;
                }

                break;
        }

        /* increments the index value (the counter
        for the file cursor position) */
        index++;
    }

    /* releases the file buffer */
    free(fileBuffer);
}

char _getcTemplateEngine(FILE *file, unsigned char *fileBuffer, size_t *index) {
    /* retrieves the current character
    from the file stream */
    char current = getc(file);

    /* in case the current retrieved character
    is an end of file nothing should be updated */
    if(current == EOF) {
        /* returns the character immediately */
        return current;
    }

    /* updates the file buffer with the current
    character and increments the index reference */
    fileBuffer[*index] = current;
    *index++;

    /* returns the current character */
    return current;
}
