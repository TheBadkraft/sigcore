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
 * File: strings.c
 * Description: Implementation of string utilities in SigmaCore
 *
 * MEMORY ALLOCATION POLICY:
 * All memory allocations must use scope_alloc() for proper scope management.
 * When standard library functions allocate memory (e.g., strdup, asprintf),
 * the allocated memory must be imported using scope_import() and the original
 * system allocation freed immediately.
 */

#include "sigcore/strings.h"
#include "internal/collections.h"
#include "internal/memory_internal.h"
#include "sigcore/collections.h"
#include "sigcore/memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ======================================================================== */
/* String Functions                                                         */
/* ======================================================================== */

// Helper function definitions
// Allocate and copy a string of given length
static string string_alloc_copy(const char *src, size_t len) {
   if (!src || len == 0)
      return NULL;

   string result = scope_alloc(len + 1, false);
   if (result) {
      memcpy(result, src, len);
      result[len] = '\0';
   }
   return result;
}

/* Returns the length of a string */
size_t string_length(const string str) {
   return str ? strlen(str) : 0;
}

/* Returns a copy of given string */
string string_copy(const string str) {
   return string_alloc_copy(str, string_length(str));
}

/* Returns a duplicate string */
string string_duplicate(const char *str) {
   return string_alloc_copy(str, str ? strlen(str) : 0);
}

/* Returns a concatenated string */
string string_concat(const string str1, const string str2) {
   if (!str1 || !str2)
      return NULL;

   size_t len1 = strlen(str1);
   size_t len2 = strlen(str2);
   string result = scope_alloc(len1 + len2 + 1, false);
   if (result) {
      strcpy(result, str1);
      strcpy(result + len1, str2);
   }
   return result;
}

/* Compares two strings */
int string_compare(const string str1, const string str2) {
   if (str1 == str2)
      return 0;
   if (!str1 || !str2)
      return str1 ? 1 : -1;
   return strcmp(str1, str2);
}

/* Returns a formatted string */
string string_format(const string format, ...) {
   if (!format)
      return NULL;
   va_list args;
   va_start(args, format);
   int len = vsnprintf(NULL, 0, format, args);
   va_end(args);
   if (len < 0)
      return NULL;

   string result = scope_alloc(len + 1, false);
   if (!result)
      return NULL;
   va_start(args, format);
   vsnprintf(result, len + 1, format, args);
   va_end(args);
   return result;
}

/* Dispose string from allocated memory */
void string_dispose(string str) {
   if (str)
      Memory.dispose(str);
}

char *string_to_array(string str) {
   return string_alloc_copy(str, string_length(str));
}

const sc_string_i String = {
    .length = string_length,
    .copy = string_copy,
    .dupe = string_duplicate,
    .concat = string_concat,
    .compare = string_compare,
    .format = string_format,
    .to_array = string_to_array,
    .dispose = string_dispose,
};

/* ======================================================================== */
/* StringBuilder Implementation                                             */
/* ======================================================================== */
struct string_builder_s {
   farray array;    /* The underlying farray for storage */
   char *buffer;    /* Direct pointer to the buffer for efficiency */
   size_t capacity; /* Current buffer capacity */
   size_t length;   /* Current string length (excluding null terminator) */
};

/* Initializes a string builder with the given capacity */
string_builder stringbuilder_new(size_t capacity) {
   if (capacity == 0)
      capacity = 16;
   string_builder sb = scope_alloc(sizeof(struct string_builder_s), false);
   if (!sb)
      return NULL;

   sb->array = FArray.new(capacity + 1, 1); /* +1 for null terminator */
   if (!sb->array) {
      Memory.dispose(sb);
      return NULL;
   }

   /* Get the buffer pointer */
   collection coll = FArray.as_collection(sb->array, 1);
   sb->buffer = (char *)collection_get_buffer(coll);
   sb->capacity = capacity;
   sb->length = 0;
   sb->buffer[0] = '\0';
   Collections.dispose(coll);
   return sb;
}

/* Initializes a new string builder from char* buffer. */
string_builder stringbuilder_from_string(string str) {
   if (!str)
      return stringbuilder_new(0);
   size_t len = strlen(str);
   string_builder sb = stringbuilder_new(len + 1);
   if (sb)
      stringbuilder_append(sb, str);
   return sb;
}

/* Appends a plain string to the buffer, resizing if necessary */
void stringbuilder_append(string_builder sb, string str) {
   if (!sb || !str)
      return;
   size_t len = strlen(str);
   size_t needed_capacity = sb->length + len + 1;

   if (needed_capacity > sb->capacity) {
      size_t new_capacity = needed_capacity;

      farray new_array = FArray.new(new_capacity, 1);
      if (!new_array)
         return;

      collection new_coll = FArray.as_collection(new_array, 1);
      char *new_buffer = (char *)collection_get_buffer(new_coll);
      memcpy(new_buffer, sb->buffer, sb->length + 1);
      Collections.dispose(new_coll);

      FArray.dispose(sb->array);
      sb->array = new_array;
      sb->buffer = new_buffer;
      sb->capacity = new_capacity - 1;
   }

   memcpy(sb->buffer + sb->length, str, len);
   sb->length += len;
   sb->buffer[sb->length] = '\0';
}

/* Appends a formatted string */
void stringbuilder_appendf(string_builder sb, string format, ...) {
   if (!sb || !format)
      return;
   va_list args;
   va_start(args, format);
   va_list args_copy;
   va_copy(args_copy, args);
   int len = vsnprintf(NULL, 0, format, args_copy);
   va_end(args_copy);
   if (len < 0) {
      va_end(args);
      return;
   }

   size_t required_len = (size_t)len;
   size_t needed_capacity = sb->length + required_len + 1;

   if (needed_capacity > sb->capacity) {
      size_t new_capacity = needed_capacity;

      farray new_array = FArray.new(new_capacity, 1);
      if (!new_array) {
         va_end(args);
         return;
      }

      collection new_coll = FArray.as_collection(new_array, 1);
      char *new_buffer = (char *)collection_get_buffer(new_coll);
      memcpy(new_buffer, sb->buffer, sb->length + 1);
      Collections.dispose(new_coll);

      FArray.dispose(sb->array);
      sb->array = new_array;
      sb->buffer = new_buffer;
      sb->capacity = new_capacity - 1;
   }

   vsnprintf(sb->buffer + sb->length, required_len + 1, format, args);
   sb->length += required_len;
   va_end(args);
}

/* Appends a string followed by a newline */
void stringbuilder_appendl(string_builder sb, string str) {
   if (!sb)
      return;
   if (str)
      stringbuilder_append(sb, str);
   stringbuilder_append(sb, "\n");
}

/* Appends a newline followed by the string */
void stringbuilder_lappends(string_builder sb, string str) {
   if (!sb)
      return;
   stringbuilder_append(sb, "\n");
   if (str)
      stringbuilder_append(sb, str);
}

/* Appends a newline followed by a formatted string */
void stringbuilder_lappendf(string_builder sb, string format, ...) {
   if (!sb || !format)
      return;
   stringbuilder_append(sb, "\n");
   va_list args;
   va_start(args, format);
   // Simplified: just append the formatted string
   char buffer[1024];
   vsnprintf(buffer, sizeof(buffer), format, args);
   stringbuilder_append(sb, buffer);
   va_end(args);
}

/* Resets the buffer to empty */
void stringbuilder_clear(string_builder sb) {
   if (!sb)
      return;
   sb->length = 0;
   sb->buffer[0] = '\0';
}

/* Returns a new string with the current content */
string stringbuilder_to_string(string_builder sb) {
   if (!sb || !sb->buffer)
      return NULL;
   size_t len = stringbuilder_length(sb);
   string result = scope_alloc(len + 1, false);
   if (result)
      strcpy(result, sb->buffer);
   return result;
}

/* Writes the buffer contents to the given stream */
void stringbuilder_to_stream(string_builder sb, FILE *stream) {
   if (!sb || !sb->buffer || !stream)
      return;
   size_t len = stringbuilder_length(sb);
   if (len > 0)
      fwrite(sb->buffer, 1, len, stream);
}

/* Returns the current number of characters */
size_t stringbuilder_length(string_builder sb) {
   return sb ? sb->length : 0;
}

/* Returns the total capacity */
size_t stringbuilder_capacity(string_builder sb) {
   if (!sb)
      return 0;
   return FArray.capacity(sb->array, 1) - 1; /* -1 for null terminator */
}

/* Adjusts the buffer capacity */
void stringbuilder_set_capacity(string_builder sb, size_t new_capacity) {
   if (!sb || new_capacity <= sb->capacity)
      return;

   /* Force farray to grow to the new capacity */
   char dummy = '\0';
   FArray.set(sb->array, new_capacity, 1, &dummy);

   /* Update buffer pointer and capacity */
   collection coll = FArray.as_collection(sb->array, 1);
   sb->buffer = (char *)collection_get_buffer(coll);
   sb->capacity = FArray.capacity(sb->array, 1) - 1;
   Collections.dispose(coll);
}

/* Disposes the string builder */
void stringbuilder_dispose(string_builder sb) {
   if (!sb)
      return;
   if (sb->array)
      FArray.dispose(sb->array);
   Memory.dispose(sb);
}

const sc_stringbuilder_i StringBuilder = {
    .new = stringbuilder_new,
    .snew = stringbuilder_from_string,
    .append = stringbuilder_append,
    .appendf = stringbuilder_appendf,
    .appendl = stringbuilder_appendl,
    .lappends = stringbuilder_lappends,
    .lappendf = stringbuilder_lappendf,
    .clear = stringbuilder_clear,
    .toString = stringbuilder_to_string,
    .toStream = stringbuilder_to_stream,
    .length = stringbuilder_length,
    .capacity = stringbuilder_capacity,
    .setCapacity = stringbuilder_set_capacity,
    .dispose = stringbuilder_dispose,
};