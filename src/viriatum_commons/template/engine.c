/*
 Hive Viriatum Commons
 Copyright (c) 2008-2026 Hive Solutions Lda.

 This file is part of Hive Viriatum Commons.

 Hive Viriatum Commons is free software: you can redistribute it and/or modify
 it under the terms of the Apache License as published by the Apache
 Foundation, either version 2.0 of the License, or (at your option) any
 later version.

 Hive Viriatum Commons is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 Apache License for more details.

 You should have received a copy of the Apache License along with
 Hive Viriatum Commons. If not, see <http://www.apache.org/licenses/>.

 __author__    = João Magalhães <joamag@hive.pt>
 __copyright__ = Copyright (c) 2008-2026 Hive Solutions Lda.
 __license__   = Apache License, Version 2.0
*/

#include "stdafx.h"

#include "engine.h"

void create_template_engine(struct template_engine_t **template_engine_pointer) {
    /* retrieves the template engine size */
    size_t template_engine_size = sizeof(struct template_engine_t);

    /* allocates space for the template engine */
    struct template_engine_t *template_engine = (struct template_engine_t *) MALLOC(template_engine_size);

    /* sets the default values */
    template_engine->context = NULL;

    /* sets the template engine in the template engine pointer */
    *template_engine_pointer = template_engine;
}

void delete_template_engine(struct template_engine_t *template_engine) {
    /* releases the template engine */
    FREE(template_engine);
}

void create_template_settings(struct template_settings_t **template_settings_pointer) {
    /* retrieves the template settings size */
    size_t template_settings_size = sizeof(struct template_settings_t);

    /* allocates space for the template settings */
    struct template_settings_t *template_settings = (struct template_settings_t *) MALLOC(template_settings_size);

    /* sets the template settings callback values */
    template_settings->on_text_begin = NULL;
    template_settings->on_text_end = NULL;
    template_settings->on_tag_begin = NULL;
    template_settings->on_tag_close_begin = NULL;
    template_settings->on_tag_end = NULL;
    template_settings->on_tag_name = NULL;
    template_settings->on_parameter = NULL;
    template_settings->on_parameter_value = NULL;

    /* sets the template settings in the template settings pointer */
    *template_settings_pointer = template_settings;
}

void delete_template_settings(struct template_settings_t *template_settings) {
    /* releases the template settings */
    FREE(template_settings);
}

ERROR_CODE process_template_engine(struct template_engine_t *template_engine, struct template_settings_t *template_settings, unsigned char *file_path) {
    /* allocates space for the file */
    FILE *file;

    /* allocates the space for the variable that will hold
    the size of the file to be parsed */
    size_t file_size;

    /* allocates the buffer that will hold the contents
    of the read file, this buffer is released upon the
    end of the parsing */
    unsigned char *file_buffer;

    /* allocates space for the number of bytes that the reading
    of the file hands back and for the result of the parsing */
    size_t number_bytes;
    ERROR_CODE error_code;

    /* opens the file */
    FOPEN(&file, (char *) file_path, "rb");

    /* in case the file is not correctly loaded */
    if(file == NULL) {
        /* returns immediately (no file found) */
        RAISE_NO_ERROR;
    }

    /* retrieves the size of the file by seeking to the
    end of it and the seeks the stream back to the initial
    position (for further reading) */
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    /* allocates the buffer that will hold the complete
    template file (this allocation may be giant) and reads
    the file into it whole, a single operation where the
    walking of the stream a character at a time costs a
    call into the library for every one of them */
    file_buffer = (unsigned char *) MALLOC(file_size);
    number_bytes = fread(file_buffer, 1, file_size, file);

    /* closes the file, the contents of it are in memory */
    fclose(file);

    /* in case the number of read bytes is not the same as
    the total bytes in file the parser would run over what
    the buffer happens to hold past them (error) */
    if(number_bytes != file_size) {
        FREE(file_buffer);
        RAISE_ERROR_M(
            RUNTIME_EXCEPTION_ERROR_CODE,
            (unsigned char *) "Problem reading from file"
        );
    }

    /* runs the parser over the buffer that holds the file and
    releases the buffer, whatever the parsing came to */
    error_code = process_buffer_template_engine(
        template_engine,
        template_settings,
        file_buffer,
        file_size
    );
    FREE(file_buffer);

    /* in case the parsing raised an error it is raised again
    now that the buffer has been released */
    if(IS_ERROR_CODE(error_code)) { RAISE_AGAIN(error_code); }

    /* raise no error */
    RAISE_NO_ERROR;
}

ERROR_CODE process_buffer_template_engine(struct template_engine_t *template_engine, struct template_settings_t *template_settings, unsigned char *buffer, size_t size) {
    /* allocates space for the current (character) for reading
    and for the ahead character for look ahead */
    char current = '\0';
    char ahead = '\0';

    /* allocates the space for the look ahead valid flag */
    unsigned char ahead_set = 0;

    /* allocates the mark variables used to locate the part
    of context changing during the parsing, the pointer walks
    the buffer and the marks point into it, so that nothing
    is ever copied out of it during the parsing */
    unsigned char *pointer = buffer;
    unsigned char *text_end_mark = 0;
    unsigned char *tag_end_mark = 0;
    unsigned char *tag_name_mark = 0;
    unsigned char *parameter_mark = 0;
    unsigned char *parameter_value_mark = 0;

    /* allocates and starts the state for the template parsing */
    enum template_engine_state_e state = TEMPLATE_ENGINE_NORMAL;

    TEMPLATE_MARK(text_end);
    TEMPLATE_CALLBACK(text_begin);

    /* iterates continuously too run the parser
    over the complete set of buffer contents */
    while(TRUE) {
        /* in case the look ahead mode is set, should
        read from the look ahead instead of the normal
        buffer reading */
        if(ahead_set) {
            /* sets the current read character as the look
            ahead character and unsets the ahead set flag */
            current = ahead;
            ahead_set = 0;
        }
        /* otherwise it must be a normal reading */
        else {
            /* in case nothing remains in the buffer the parsing
            is over, every character of it has been through the
            parser by now, the last one of them included */
            if(size == 0) {
                /* breaks the cycle (end of parsing) */
                break;
            }

            /* retrieves the current character
            from the buffer */
            current = _getc_template_engine(&pointer, &size);
        }

        /* switches over the state to determine the appropriate
        handling to be made for the current character */
        switch(state) {
            case TEMPLATE_ENGINE_NORMAL:
                if(current == '$') {
                    state = TEMPLATE_ENGINE_DOLLAR;
                }

                break;

            case TEMPLATE_ENGINE_DOLLAR:
                if(current == '{') {
                    /* marks the tag element and calls
                    the text end and tag begin callbacks */
                    TEMPLATE_MARK(tag_name);
                    TEMPLATE_MARK_N(tag_end, 2);
                    TEMPLATE_CALLBACK_DATA_N(text_end, 2);
                    TEMPLATE_CALLBACK(tag_begin);

                    /* changes the state of the parser
                    to open (tag open) */
                    state = TEMPLATE_ENGINE_OPEN;

                    /* reads ahead and sets the ahead set flag */
                    ahead = _getc_template_engine(&pointer, &size);
                    ahead_set = 1;

                    if(ahead == '/') {
                        TEMPLATE_CALLBACK(tag_close_begin);
                        ahead_set = 0;
                    }
                } else {
                    /* resets the state to the "normal" */
                    state = TEMPLATE_ENGINE_NORMAL;
                }

                break;

            case TEMPLATE_ENGINE_OPEN:
                if(current == '/') {
                    /* reads ahead and sets the ahead set flag */
                    ahead = _getc_template_engine(&pointer, &size);
                    ahead_set = 1;

                    if(ahead == '}') {
                        state = TEMPLATE_ENGINE_NORMAL;

                        /* unsets the ahead set flag */
                        ahead_set = 0;

                        /* marks the text end */
                        TEMPLATE_MARK(text_end);

                        /* calls the tag end and text begin callbacks */
                        TEMPLATE_CALLBACK_DATA(tag_end);
                        TEMPLATE_CALLBACK(text_begin);

                        break;
                    }
                }

                if(current == '}') {
                    state = TEMPLATE_ENGINE_NORMAL;

                    TEMPLATE_MARK(text_end);

                    /* calls the tag end and text begin callbacks */
                    TEMPLATE_CALLBACK_DATA(tag_end);
                    TEMPLATE_CALLBACK(text_begin);

                    break;
                }

                if(current == ' ') {
                    /* calls the tag name callback */
                    TEMPLATE_CALLBACK_DATA_BACK(tag_name);

                    /* changes the state of the template engine
                    to parameters (parameters finding) */
                    state = TEMPLATE_ENGINE_PARAMETERS;
                }

                break;

            case TEMPLATE_ENGINE_PARAMETERS:
                if(current == '/') {
                    ahead = _getc_template_engine(&pointer, &size);
                    ahead_set = 1;

                    if(ahead == '}') {
                        state = TEMPLATE_ENGINE_NORMAL;
                        ahead_set = 0;

                        TEMPLATE_MARK(text_end);

                        /* calls the tag end and text begin callbacks */
                        TEMPLATE_CALLBACK_DATA(tag_end);
                        TEMPLATE_CALLBACK(text_begin);

                        break;
                    }
                }

                if(current == '}') {
                    state = TEMPLATE_ENGINE_NORMAL;

                    TEMPLATE_MARK(text_end);

                    /* calls the tag end and text begin callbacks */
                    TEMPLATE_CALLBACK_DATA(tag_end);
                    TEMPLATE_CALLBACK(text_begin);

                    break;
                }

                if(current != ' ') {
                    TEMPLATE_MARK_BACK(parameter);

                    state = TEMPLATE_ENGINE_PARAMETER;
                }

                break;

            case TEMPLATE_ENGINE_PARAMETER:
                if(current == '/') {
                    ahead = _getc_template_engine(&pointer, &size);
                    ahead_set = 1;

                    if(ahead == '}') {
                        state = TEMPLATE_ENGINE_NORMAL;
                        ahead_set = 0;

                        TEMPLATE_MARK(text_end);

                        /* calls the tag end and text begin callbacks */
                        TEMPLATE_CALLBACK_DATA(tag_end);
                        TEMPLATE_CALLBACK(text_begin);

                        break;
                    }
                }

                if(current == '}') {
                    state = TEMPLATE_ENGINE_NORMAL;

                    TEMPLATE_MARK(text_end);

                    /* calls the tag end and text begin callbacks */
                    TEMPLATE_CALLBACK_DATA(tag_end);
                    TEMPLATE_CALLBACK(text_begin);

                    break;
                }

                if(current == '=') {
                    /* calls the parameter callback and marks
                    the template engine parameter value */
                    TEMPLATE_CALLBACK_DATA_BACK(parameter);
                    TEMPLATE_MARK(parameter_value);

                    state = TEMPLATE_ENGINE_PARAMETER_VALUE;
                }

                break;

            case TEMPLATE_ENGINE_PARAMETER_VALUE:
                if(current == '/') {
                    ahead = _getc_template_engine(&pointer, &size);
                    ahead_set = 1;

                    if(ahead == '}') {
                        state = TEMPLATE_ENGINE_NORMAL;
                        ahead_set = 0;

                        TEMPLATE_MARK(text_end);

                        /* calls the tag end and text begin callbacks */
                        TEMPLATE_CALLBACK_DATA(tag_end);
                        TEMPLATE_CALLBACK(text_begin);

                        break;
                    }
                }

                if(current == '}') {
                    state = TEMPLATE_ENGINE_NORMAL;

                    TEMPLATE_MARK(text_end);

                    /* calls the th parameter value, tag end and
                    text begin callbacks */
                    TEMPLATE_CALLBACK_DATA_BACK(parameter_value);
                    TEMPLATE_CALLBACK_DATA(tag_end);
                    TEMPLATE_CALLBACK(text_begin);

                    break;
                }

                if(current == '\"') {
                    state = TEMPLATE_ENGINE_PARAMETER_VALUE_STRING;
                } else if(current == ' ') {
                    /* calls the parameter value callback */
                    TEMPLATE_CALLBACK_DATA_BACK(parameter_value);

                    state = TEMPLATE_ENGINE_PARAMETERS;
                }

                break;

            case TEMPLATE_ENGINE_PARAMETER_VALUE_STRING:
                if(current == '\"') {
                    /* calls the parameter value callback */
                    TEMPLATE_CALLBACK_DATA(parameter_value);

                    state = TEMPLATE_ENGINE_PARAMETERS;
                }

                break;
        }
    }

    /* in case the current state is engine
    normal (there must be text to be flushed) */
    if(state == TEMPLATE_ENGINE_NORMAL) {
        /* calls the text end callback */
        TEMPLATE_CALLBACK_DATA(text_end);
    }

    /* raise no error */
    RAISE_NO_ERROR;
}

char _getc_template_engine(unsigned char **pointer, size_t *size) {
    /* allocates space for the current character
    to be retrieve in the function */
    char current;

    /* in case the size is already zero (nothing remaining)
    nothing is read past the end of the buffer and the pointer
    is left exactly where it stands */
    if(*size == 0) {
        /* returns invalid */
        return EOF;
    }

    /* retrieves the current character from the buffer
    and increments the pointer reference past it */
    current = (char) **pointer;
    (*pointer)++;

    /* decrements the (remaining) size */
    (*size)--;

    /* returns the current character */
    return current;
}
