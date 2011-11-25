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

/**
 * Structure defining the attribute internal
 * attributes and the references to the value.
 */
typedef struct TemplateAttribute_t {
    /**
     * Teh data type of the current attribute.
     */
    int type;

    /**
     * The name of the current attribute.
     */
    unsigned char *name;

    /**
     * The value as a string of the attribute.
     */
    unsigned char *stringValue;

    /**
     * The value as a refernce of the attribute.
     */
    unsigned char *referenceValue;

    /**
     * The value as an integer of the attribute.
     */
    int intValue;

    /**
     * The value as a float of the attribute.
     */
    float floatValue;
} TemplateAttribute;

typedef struct TemplateNode_t {
    size_t childCount;
    size_t attributeCount;
    struct TemplateNode_t *children;
    struct TemplateAttribute_t *attributes;
} TemplateNode;
