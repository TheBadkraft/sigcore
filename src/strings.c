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
 */

#include "sigcore/strings.h"
#include "sigcore/collections.h"
#include "internal/collections.h"
#include "sigcore/memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Returns the length of a string */
size_t str_get_length(const string str) {
   return str ? strlen(str) : 0;
}

/* Returns a copy of given string */
string str_copy(const string str) {
   size_t len = str_get_length(str);
   string copy = NULL;
   if (len > 0) {
      copy = Memory.alloc(len + 1, false);
      if (copy)
         strcpy(copy, str);
   }
   return copy;
}

/* Returns a duplicate string */
string str_duplicate(const char *str) {
   if (!str)
      return NULL;
   size_t len = strlen(str);
   string dup = Memory.alloc(len + 1, false);
   if (dup)
      strcpy(dup, str);
   return dup;
}

/* Returns a concatenated string */
string str_concat(const string str1, const string str2) {
   if (!str1 || !str2)
      return NULL;

   size_t len1 = strlen(str1);
   size_t len2 = strlen(str2);
   string result = Memory.alloc(len1 + len2 + 1, false);
   if (result) {
      strcpy(result, str1);
      strcpy(result + len1, str2);
   }
   return result;
}

/* Compares two strings */
int str_compare(const string str1, const string str2) {
   if (str1 == str2)
      return 0;
   if (!str1 || !str2)
      return str1 ? 1 : -1;
   return strcmp(str1, str2);
}

/* Returns a formatted string */
string str_format(const string format, ...) {
   if (!format)
      return NULL;
   va_list args;
   va_start(args, format);
   int len = vsnprintf(NULL, 0, format, args);
   va_end(args);
   if (len < 0)
      return NULL;

   string result = Memory.alloc(len + 1, false);
   if (!result)
      return NULL;
   va_start(args, format);
   vsnprintf(result, len + 1, format, args);
   va_end(args);
   return result;
}

/* Dispose string from allocated memory */
void str_dispose(string str) {
   if (str)
      Memory.dispose(str);
}

char *str_to_array(string str) {
   if (!str)
      return NULL;
   size_t len = str_get_length(str);
   char *arr = Memory.alloc(len + 1, false);
   if (!arr)
      return NULL;
   memcpy(arr, str, len + 1);
   return arr;
}

const sc_string_i String = {
    .length = str_get_length,
    .copy = str_copy,
    .dupe = str_duplicate,
    .concat = str_concat,
    .compare = str_compare,
    .format = str_format,
    .to_array = str_to_array,
    .dispose = str_dispose,
};

/* String Builder Implementation */
struct string_builder_s {
   farray array;    /* The underlying farray for storage */
   char *buffer;    /* Direct pointer to the buffer for efficiency */
   size_t capacity; /* Current buffer capacity */
   size_t length;   /* Current string length (excluding null terminator) */
};

/* Initializes a string builder with the given capacity */
string_builder sb_new(size_t capacity) {
   if (capacity == 0)
      capacity = 16;
   string_builder sb = Memory.alloc(sizeof(struct string_builder_s), false);
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
string_builder sb_from_string(string str) {
   if (!str)
      return sb_new(0);
   size_t len = strlen(str);
   string_builder sb = sb_new(len + 1);
   if (sb)
      sb_append(sb, str);
   return sb;
}

/* Appends a plain string to the buffer, resizing if necessary */
void sb_append(string_builder sb, string str) {
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
void sb_appendf(string_builder sb, string format, ...) {
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
void sb_appendl(string_builder sb, string str) {
   if (!sb)
      return;
   if (str)
      sb_append(sb, str);
   sb_append(sb, "\n");
}

/* Appends a newline followed by the string */
void sb_lappends(string_builder sb, string str) {
   if (!sb)
      return;
   sb_append(sb, "\n");
   if (str)
      sb_append(sb, str);
}

/* Appends a newline followed by a formatted string */
void sb_lappendf(string_builder sb, string format, ...) {
   if (!sb || !format)
      return;
   sb_append(sb, "\n");
   va_list args;
   va_start(args, format);
   // Simplified: just append the formatted string
   char buffer[1024];
   vsnprintf(buffer, sizeof(buffer), format, args);
   sb_append(sb, buffer);
   va_end(args);
}

/* Resets the buffer to empty */
void sb_clear(string_builder sb) {
   if (!sb)
      return;
   sb->length = 0;
   sb->buffer[0] = '\0';
}

/* Returns a new string with the current content */
string sb_to_string(string_builder sb) {
   if (!sb || !sb->buffer)
      return NULL;
   size_t len = sb_get_length(sb);
   string result = Memory.alloc(len + 1, false);
   if (result)
      strcpy(result, sb->buffer);
   return result;
}

/* Writes the buffer contents to the given stream */
void sb_to_stream(string_builder sb, FILE *stream) {
   if (!sb || !sb->buffer || !stream)
      return;
   size_t len = sb_get_length(sb);
   if (len > 0)
      fwrite(sb->buffer, 1, len, stream);
}

/* Returns the current number of characters */
size_t sb_get_length(string_builder sb) {
   return sb ? sb->length : 0;
}

/* Returns the total capacity */
size_t sb_get_capacity(string_builder sb) {
   if (!sb)
      return 0;
   return FArray.capacity(sb->array, 1) - 1; /* -1 for null terminator */
}

/* Adjusts the buffer capacity */
void sb_set_capacity(string_builder sb, size_t new_capacity) {
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
void sb_dispose(string_builder sb) {
   if (!sb)
      return;
   if (sb->array)
      FArray.dispose(sb->array);
   Memory.dispose(sb);
}

const sc_stringbuilder_i StringBuilder = {
    .new = sb_new,
    .snew = sb_from_string,
    .append = sb_append,
    .appendf = sb_appendf,
    .appendl = sb_appendl,
    .lappends = sb_lappends,
    .lappendf = sb_lappendf,
    .clear = sb_clear,
    .toString = sb_to_string,
    .toStream = sb_to_stream,
    .length = sb_get_length,
    .capacity = sb_get_capacity,
    .setCapacity = sb_set_capacity,
    .dispose = sb_dispose,
};