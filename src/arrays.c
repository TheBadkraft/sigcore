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
 * File: arrays.c
 * Description: Common array operations implementation
 */
#include "internal/arrays.h"
#include "internal/memory_internal.h"
#include "sigcore/memory.h"
#include <string.h>

// allocate memory for an array bucket
object array_alloc_bucket(size_t element_size, usize capacity) {
   // Check for overflow: capacity * element_size > SIZE_MAX
   if (capacity > 0 && element_size > SIZE_MAX / capacity) {
      return NULL; // Would overflow
   }
   return scope_alloc(element_size * capacity, false);
}

// free array resources (bucket and struct)
void array_free_resources(void *bucket, void *struct_ptr) {
   if (bucket) {
      Memory.dispose(bucket);
   }
   if (struct_ptr) {
      Memory.dispose(struct_ptr);
   }
}

// Common array structure allocation with bucket
// Handles the common pattern: allocate struct, set handle, allocate bucket, set end, check overflow
void *array_alloc_struct_with_bucket(usize struct_size, char handle_char,
                                     usize element_size, usize capacity,
                                     void **bucket_out, char **end_out) {
   // Allocate memory for the array structure
   void *struct_ptr = scope_alloc(struct_size, false);
   if (!struct_ptr) {
      return NULL;
   }

   // Set type handle
   ((char *)struct_ptr)[0] = handle_char;
   ((char *)struct_ptr)[1] = '\0';

   // Allocate memory for the bucket
   void *bucket = array_alloc_bucket(element_size, capacity);
   if (!bucket && capacity > 0) {
      Memory.dispose(struct_ptr);
      return NULL;
   }

   // Set end pointer
   char *end = (char *)bucket + element_size * capacity;

   // Check for pointer arithmetic overflow
   if (capacity > 0 && end < (char *)bucket) {
      array_free_resources(bucket, struct_ptr);
      return NULL;
   }

   // Return results
   if (bucket_out)
      *bucket_out = bucket;
   if (end_out)
      *end_out = end;

   return struct_ptr;
}