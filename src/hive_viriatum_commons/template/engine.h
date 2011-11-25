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

#include "../debug/debug.h"
#include "../system/system.h"

#define TEMPLATE_MARK(FOR)\
    do {\
        FOR##Mark = pointer;\
    } while(0)

#define TEMPLATE_MARK_BACK(FOR)\
    do {\
        FOR##Mark = pointer - 1;\
    } while(0)

#define TEMPLATE_MARK_N(FOR, N)\
    do {\
        FOR##Mark = pointer - N;\
    } while(0)

#define TEMPLATE_CALLBACK(FOR)\
    do {\
        if(templateSettings->on##FOR) {\
            if(templateSettings->on##FOR(templateEngine) != 0) {\
                /*SET_ERRNO(HPE_CB_##FOR);*/\
                /*return (pointer - data);*/\
            }\
        }\
    } while(0)

#define TEMPLATE_CALLBACK_DATA(FOR)\
    do {\
        if(FOR##Mark) {\
            if(templateSettings->on##FOR) {\
                if(templateSettings->on##FOR(templateEngine, FOR##Mark, pointer - FOR##Mark) != 0) {\
                /*    SET_ERRNO(HPE_CB_##FOR);*/\
                    /*return (p - data);*/\
                }\
            }\
            FOR##Mark = NULL;\
        }\
    } while(0)

#define TEMPLATE_CALLBACK_DATA_BACK(FOR)\
    do {\
        if(FOR##Mark) {\
            if(templateSettings->on##FOR) {\
                if(templateSettings->on##FOR(templateEngine, FOR##Mark, pointer - FOR##Mark - 1) != 0) {\
                /*    SET_ERRNO(HPE_CB_##FOR);*/\
                    /*return (p - data);*/\
                }\
            }\
            FOR##Mark = NULL;\
        }\
    } while(0)

struct TemplateEngine_t;

/**
 * The "default" callback function to be used, without
 * any extra arguments.
 */
typedef ERROR_CODE (*templateCallback) (struct TemplateEngine_t *);

/**
 * Callback function type used for callbacks that require
 * "extra" data to be send as argument.
 */
typedef ERROR_CODE (*templateDataCallback) (struct TemplateEngine_t *, const unsigned char *, size_t);

typedef enum TemplateEngineState_e {
    /**
     * Normal state for the template engin where
     * it is trying to find new tags.
     */
    TEMPLATE_ENGINE_NORMAL = 1,

    /**
     * State where the tempalte engine has just found
     * a dollar sign and is trying to open a tage with an
     * open braces chracter.
     */
    TEMPLATE_ENGINE_DOLAR,

    /**
     * Sate when the tag has been open and the parser
     * is trying to find the name of the tag.
     */
    TEMPLATE_ENGINE_OPEN,

    /**
     * State for the context where the template
     * engine is trying to find the initial part
     * of a "new" parameter inside the tag.
     */
    TEMPLATE_ENGINE_PARAMETERS,

    /**
     * State when a new parameter has just been found
     * and the parser is trying to find the limit of it.
     */
    TEMPLATE_ENGINE_PARAMETER,

    /**
     * State when the initial part of a value has been
     * found and the parser is trying to find the end
     * of the value.
     */
    TEMPLATE_ENGINE_PARAMETER_VALUE,

    /**
     * State where the initial part of a string based value
     * has been found and the end of string is trying to be
     * found for complete string value assert.
     */
    TEMPLATE_ENGINE_PARAMETER_VALUE_STRING
} TemplateEngineState;

/**
 * Structure representing the various settings
 * to be used for (and during) parsing the template.
 */
typedef struct TemplateSettings_t {
    /**
     * Callabck function called when a new tag
     * is opened.
     */
    templateCallback ontagBegin;

    /**
     * Callback function called when a new close
     * type tag is opened.
     */
    templateCallback ontagCloseBegin;

    /**
     * Callback function called when an existing
     * tag is closed.
     */
    templateDataCallback ontagEnd;

    /**
     * Callback function called when a name of a tag
     * is parsed correctly.
     */
    templateDataCallback ontagName;

    /**
     * Callback function called when a parameter name
     * is parsed correctly.
     */
    templateDataCallback onparameter;

    /**
     * Callback function called when a parameter value
     * is parsed correctly.
     */
    templateDataCallback onparameterValue;
} TemplateSettings;

/**
 * Structure repsenting the template engine (parser)
 * used to store all the context of the current
 * parsing.
 * Additional options may be used to store context
 * information that can be used in the callbacks.
 */
typedef struct TemplateEngine_t {
    /**
     * Context information to be used by the parsing
     * handlers (callbacks) for context.
     */
    void *context;
} TemplateEngine;

VIRIATUM_EXPORT_PREFIX void createTemplateEngine(struct TemplateEngine_t **templateEnginePointer);
VIRIATUM_EXPORT_PREFIX void deleteTemplateEngine(struct TemplateEngine_t *templateEngine);
VIRIATUM_EXPORT_PREFIX void createTemplateSettings(struct TemplateSettings_t **templateSettingsPointer);
VIRIATUM_EXPORT_PREFIX void deleteTemplateSettings(struct TemplateSettings_t *templateSettings);
VIRIATUM_EXPORT_PREFIX void processTemplateEngine(struct TemplateEngine_t *templateEngine, struct TemplateSettings_t *templateSettings, unsigned char *filePath);
VIRIATUM_EXPORT_PREFIX char _getcTemplateEngine(FILE *file, unsigned char **pointer);
