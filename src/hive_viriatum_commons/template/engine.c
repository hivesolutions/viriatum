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

    /* sets the default values */
    templateEngine->context = NULL;

    /* sets the template engine in the template engine pointer */
    *templateEnginePointer = templateEngine;
}

void deleteTemplateEngine(struct TemplateEngine_t *templateEngine) {
    /* releases the template engine */
    FREE(templateEngine);
}

void createTemplateSettings(struct TemplateSettings_t **templateSettingsPointer) {
    /* retrieves the template settings size */
    size_t templateSettingsSize = sizeof(struct TemplateSettings_t);

    /* allocates space for the template settings */
    struct TemplateSettings_t *templateSettings = (struct TemplateSettings_t *) MALLOC(templateSettingsSize);

    /* sets the template settings callback values */
    templateSettings->ontextBegin = NULL;
    templateSettings->ontextEnd = NULL;
    templateSettings->ontagBegin = NULL;
    templateSettings->ontagCloseBegin = NULL;
    templateSettings->ontagEnd = NULL;
    templateSettings->ontagName = NULL;
    templateSettings->onparameter = NULL;
    templateSettings->onparameterValue = NULL;

    /* sets the template settings in the template settings pointer */
    *templateSettingsPointer = templateSettings;
}

void deleteTemplateSettings(struct TemplateSettings_t *templateSettings) {
    /* releases the template settings */
    FREE(templateSettings);
}

ERROR_CODE processTemplateEngine(struct TemplateEngine_t *templateEngine, struct TemplateSettings_t *templateSettings, unsigned char *filePath) {
    /* allocates space for the file */
    FILE *file;

    /* allocates space for the current (character) for reading
    and for the ahead character for look ahead */
    char current = '\0';
    char ahead = '\0';

    /* allocates the space for the look ahead valid flag */
    unsigned char aheadSet = 0;

    /* allocates the space for the variable that will hold
    the size of the file to be parsed */
    size_t fileSize;

    /* allocates the buffer thath will hold the contents
    of the read file, this buffer is realeased uppon the
    end of the parsing */
    unsigned char *fileBuffer;

    /* allocates space for the buffer that will serve as
    cache for the reading of the tempalte file, this has
    serious implications on the performance of the file */
    char _fileBuffer[ENGINE_BUFFER_SIZE];

    /* allocates the mark variables used to locate
    the part of context changing durring the parsing */
    unsigned char *pointer = 0;
    unsigned char *textEndMark = 0;
    unsigned char *tagEndMark = 0;
    unsigned char *tagNameMark = 0;
    unsigned char *parameterMark = 0;
    unsigned char *parameterValueMark = 0;

    /* allocates and starts the state for the template parsing */
    enum TemplateEngineState_e state = TEMPLATE_ENGINE_NORMAL;

    /* opens the file */
    FOPEN(&file, (char *) filePath, "rb");

    /* in case the file is not correctly loaded */
    if(file == NULL) {
        /* returns immediately (no file found) */
        RAISE_NO_ERROR;
    }

    /* then sets the buffer on the file for reading, this
    operation has serious implications in the file access
    performance (buffered reading )*/
    setvbuf(file, _fileBuffer, _IOFBF, ENGINE_BUFFER_SIZE);
    
    /* retrieves the size of the file by seeking to the
    end of it and the seeks the stream back to the initial
    position (for further reading) */
    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    /* allocates the buffer that will hold the complete
    template file (this allocation may be giant), this is
    necessary for the correct execution of the parser, uses
    the file buffer reference as the (initial) pointer value */
    fileBuffer = (void *) MALLOC(fileSize);
    pointer = fileBuffer;

    TEMPLATE_MARK(textEnd);
    TEMPLATE_CALLBACK(textBegin);

    /* iterates continuously too run the parser
    over the complete set of file contents */
    while(1) {
        /* in case the look ahead mode is set, should
        read from the look ahead instead of the normal
        file reading */
        if(aheadSet) {
            /* sets the current read character as the look
            ahead character and unsets the ahead set flag */
            current = ahead;
            aheadSet = 0;
        }
        /* otherwise it must be a normal reading */
        else {
            /* retrieves the current character
            from the file stream */
            current = _getcTemplateEngine(file, &pointer, &fileSize);
        }

        /* in case the end of file has been found, or
        the file size is zero (breaks) */
        if(current == EOF || fileSize == 0) {
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
                    /* marks the tag element and calls
                    the text end and tag begin callbacks */
                    TEMPLATE_MARK(tagName);
                    TEMPLATE_MARK_N(tagEnd, 2);
                    TEMPLATE_CALLBACK_DATA_N(textEnd, 2);
                    TEMPLATE_CALLBACK(tagBegin);

                    /* changes the state of the parser
                    to open (tag open) */
                    state = TEMPLATE_ENGINE_OPEN;

                    /* reads ahead and sets the ahead set flag */
                    ahead = _getcTemplateEngine(file, &pointer, &fileSize);
                    aheadSet = 1;

                    if(ahead == '/') {
                        TEMPLATE_CALLBACK(tagCloseBegin);
                        aheadSet = 0;
                    }
                } else {
                    /* resets the state to the "normal" */
                    state = TEMPLATE_ENGINE_NORMAL;
                }

                break;

            case TEMPLATE_ENGINE_OPEN:
                if(current == '/') {
                    /* reads ahead and sets the ahead set flag */
                    ahead = _getcTemplateEngine(file, &pointer, &fileSize);
                    aheadSet = 1;

                    if(ahead == '}') {
                        state = TEMPLATE_ENGINE_NORMAL;

                        /* unsets the ahead set flag */
                        aheadSet = 0;

                        /* marks the text end */
                        TEMPLATE_MARK(textEnd);

                        /* calls the tag end and text begin callbacks */
                        TEMPLATE_CALLBACK_DATA(tagEnd);
                        TEMPLATE_CALLBACK(textBegin);

                        break;
                    }
                }

                if(current == '}') {
                    state = TEMPLATE_ENGINE_NORMAL;

                    TEMPLATE_MARK(textEnd);

                    /* calls the tag end and text begin callbacks */
                    TEMPLATE_CALLBACK_DATA(tagEnd);
                    TEMPLATE_CALLBACK(textBegin);

                    break;
                }

                if(current == ' ') {
                    /* calls the tag name callback */
                    TEMPLATE_CALLBACK_DATA_BACK(tagName);

                    /* changes the state of the template engine
                    to parametrs (parameters finding) */
                    state = TEMPLATE_ENGINE_PARAMETERS;
                }

                break;

            case TEMPLATE_ENGINE_PARAMETERS:
                if(current == '/') {
                    ahead = _getcTemplateEngine(file, &pointer, &fileSize);
                    aheadSet = 1;

                    if(ahead == '}') {
                        state = TEMPLATE_ENGINE_NORMAL;
                        aheadSet = 0;

                        TEMPLATE_MARK(textEnd);

                        /* calls the tag end and text begin callbacks */
                        TEMPLATE_CALLBACK_DATA(tagEnd);
                        TEMPLATE_CALLBACK(textBegin);

                        break;
                    }
                }

                if(current == '}') {
                    state = TEMPLATE_ENGINE_NORMAL;

                    TEMPLATE_MARK(textEnd);

                    /* calls the tag end and text begin callbacks */
                    TEMPLATE_CALLBACK_DATA(tagEnd);
                    TEMPLATE_CALLBACK(textBegin);

                    break;
                }

                if(current != ' ') {
                    TEMPLATE_MARK_BACK(parameter);

                    state = TEMPLATE_ENGINE_PARAMETER;
                }

                break;

            case TEMPLATE_ENGINE_PARAMETER:
                if(current == '/') {
                    ahead = _getcTemplateEngine(file, &pointer, &fileSize);
                    aheadSet = 1;

                    if(ahead == '}') {
                        state = TEMPLATE_ENGINE_NORMAL;
                        aheadSet = 0;

                        TEMPLATE_MARK(textEnd);

                        /* calls the tag end and text begin callbacks */
                        TEMPLATE_CALLBACK_DATA(tagEnd);
                        TEMPLATE_CALLBACK(textBegin);

                        break;
                    }
                }

                if(current == '}') {
                    state = TEMPLATE_ENGINE_NORMAL;

                    TEMPLATE_MARK(textEnd);

                    /* calls the tag end and text begin callbacks */
                    TEMPLATE_CALLBACK_DATA(tagEnd);
                    TEMPLATE_CALLBACK(textBegin);

                    break;
                }

                if(current == '=') {
                    /* calls the parameter callback and marks
                    the template engine parameter value */
                    TEMPLATE_CALLBACK_DATA_BACK(parameter);
                    TEMPLATE_MARK(parameterValue);

                    state = TEMPLATE_ENGINE_PARAMETER_VALUE;
                }

                break;

            case TEMPLATE_ENGINE_PARAMETER_VALUE:
                if(current == '/') {
                    ahead = _getcTemplateEngine(file, &pointer, &fileSize);
                    aheadSet = 1;

                    if(ahead == '}') {
                        state = TEMPLATE_ENGINE_NORMAL;
                        aheadSet = 0;

                        TEMPLATE_MARK(textEnd);

                        /* calls the tag end and text begin callbacks */
                        TEMPLATE_CALLBACK_DATA(tagEnd);
                        TEMPLATE_CALLBACK(textBegin);

                        break;
                    }
                }

                if(current == '}') {
                    state = TEMPLATE_ENGINE_NORMAL;

                    TEMPLATE_MARK(textEnd);

                    /* calls the th parameter value, tag end and
                    text begin callbacks */
                    TEMPLATE_CALLBACK_DATA_BACK(parameterValue);
                    TEMPLATE_CALLBACK_DATA(tagEnd);
                    TEMPLATE_CALLBACK(textBegin);

                    break;
                }

                if(current == '\"') {
                    state = TEMPLATE_ENGINE_PARAMETER_VALUE_STRING;
                } else if(current == ' ') {
                    /* calls the parameter value callback */
                    TEMPLATE_CALLBACK_DATA_BACK(parameterValue);

                    state = TEMPLATE_ENGINE_PARAMETERS;
                }

                break;

            case TEMPLATE_ENGINE_PARAMETER_VALUE_STRING:
                if(current == '\"') {
                    /* calls the parameter value callback */
                    TEMPLATE_CALLBACK_DATA(parameterValue);

                    state = TEMPLATE_ENGINE_PARAMETERS;
                }

                break;
        }
    }

    /* in case the current state is engine
    normal (there must be text to be flushed) */
    if(state == TEMPLATE_ENGINE_NORMAL) {
        /* calls the text end callback */
        TEMPLATE_CALLBACK_DATA(textEnd);
    }

    /* closes the file */
    fclose(file);

    /* releases the file buffer */
    FREE(fileBuffer);

    /* raise no error */
    RAISE_NO_ERROR;
}

char _getcTemplateEngine(FILE *file, unsigned char **pointer, size_t *size) {
    /* allocates space for the current character
    to be retrieve in the function */
    char current;

    /* in case the size is already zero (nothing remaining) */
    if(size == 0) {
        /* returns invalid */
        return 0;
    }

    /* retrieves the current character
    from the file stream */
    current = getc(file);

    /* in case the current retrieved character
    is an end of file nothing should be updated */
    if(current == EOF) {
        /* returns the character immediately */
        return current;
    }

    /* updates the file buffer (from pointer) with the current
    character and increments the pointer reference */
    **pointer = current;
    (*pointer)++;

    /* decrements the file size */
    (*size)--;

    /* returns the current character */
    return current;
}
