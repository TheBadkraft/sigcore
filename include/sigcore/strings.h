/*
 * SigmaCore
 * Copyright (c) 2025 David Boarman (BadKraft) and contributors
 * QuantumOverride [Q|]
 * ----------------------------------------------
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * ----------------------------------------------
 * File: strings.h
 * Description: Header file for string utilities in SigmaCore
 */

#pragma once

#include "sigcore/collections.h"
#include "sigcore/types.h"
#include <stdarg.h>
#include <stdio.h>

/*
 * Unified utilities for string and string_builder
 */

/* Opaque string builder structure */
typedef struct string_builder_s *string_builder;

/* IString Helpers */
size_t string_length(const string);
string string_copy(const string);
string string_concat(const string, const string);
int string_compare(const string, const string);

/* IStringBuilder Helpers */
string_builder stringbuilder_new(size_t);
string_builder stringbuilder_from_string(string);
size_t stringbuilder_length(string_builder);
void stringbuilder_append(string_builder, string);
string stringbuilder_to_string(string_builder);
void stringbuilder_dispose(string_builder);

/* IString interface */
typedef struct sc_string_i {
   size_t (*length)(string);         /**< Returns the length of a string. */
   string (*copy)(string);           /**< Creates a copy of a string. */
   string (*dupe)(const char *);     /**< Duplicates a string. */
   string (*concat)(string, string); /**< Returns a concatenated string. */
   string (*format)(string, ...);    /**< Returns a formatted string. */
   int (*compare)(string, string);   /**< Compares two strings for equality. */
   char *(*to_array)(string);        /**< Returns a char array copy of the string. */
   void (*dispose)(string);          /**< Disposes the string allocation. */
} sc_string_i;

/* IStringBuilder interface */
typedef struct sc_stringbuilder_i {
   string_builder (*new)(size_t capacity);        /**< Initializes with a starting capacity. */
   string_builder (*snew)(string);                /**< Initializes a new string builder from char* buffer. */
   void (*append)(string_builder, string);        /**< Appends a string to the buffer. */
   void (*appendf)(string_builder, string, ...);  /**< Appends a formatted string using printf-style specifiers. */
   void (*appendl)(string_builder, string);       /**< Appends a string followed by a newline, or just a newline if NULL. */
   void (*lappends)(string_builder, string);      /**< Appends a newline followed by the string */
   void (*lappendf)(string_builder, string, ...); /**< Appends a newline followed by a formatted string */
   void (*clear)(string_builder);                 /**< Resets the buffer to empty, clearing content and resetting last */
   string (*toString)(string_builder);            /**< Returns a new string with the current content, caller must dispose */
   void (*toStream)(string_builder, FILE *);      /**< Writes the buffer contents to the given stream */
   size_t (*length)(string_builder);              /**< Returns the current number of characters in the buffer */
   size_t (*capacity)(string_builder);            /**< Returns the total number of usable characters in the buffer */
   void (*setCapacity)(string_builder, size_t);   /**< Adjusts the buffer capacity, preserving current content */
   void (*dispose)(string_builder);               /**< Disposes the string builder and its buffer */
} sc_stringbuilder_i;

/* Global instances */
extern const sc_string_i String;
extern const sc_stringbuilder_i StringBuilder;