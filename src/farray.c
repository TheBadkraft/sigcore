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
 * File: farray.c
 * Description: Source file for SigmaCore flex-array definitions and interfaces
 *
 * Flex-Array: One of the two core collection structures used to unify all
 *        collection types within SigmaCore. The flex-array (farray) allows storing
 *        elements of arbitrary size directly, without the overhead of pointer
 *        indirection. This is useful for small types like integers or structs
 *        where memory efficiency is important.
 */
#include "sigcore/farray.h"
#include "internal/array_base.h"
#include "internal/arrays.h"
#include "internal/collections.h"
#include "internal/memory_internal.h"
#include "sigcore/collections.h"
#include "sigcore/memory.h"
#include <string.h>

//  declare the FlexArray struct: the collection with attitude
struct sc_flex_array {
   char handle[2]; // {'F', '\0'} - type identifier
   void *bucket;   // pointer to first element (raw bytes)
   void *end;      // one past allocated memory
};

#if 1 // Region: Forward declarations
// API functions
static farray farray_new(usize, usize);
static void farray_init(farray *, usize, usize);
static void farray_dispose(farray);
static int farray_capacity(farray, usize);
static void farray_clear(farray, usize);
static int farray_set_at(farray, usize, usize, object);
static int farray_get_at(farray, usize, usize, object);
static int farray_remove_at(farray, usize, usize);

// Collection interface functions
static collection farray_as_collection(farray arr, usize stride);
static collection farray_to_collection(farray arr, usize stride);
#endif

// API function implementations
static farray farray_new(usize capacity, usize stride) {
   void *bucket;
   char *end;

   struct sc_flex_array *arr = array_alloc_struct_with_bucket(
       sizeof(struct sc_flex_array), 'F', stride, capacity, &bucket, &end);

   if (!arr) {
      return NULL;
   }

   arr->bucket = bucket;
   arr->end = end;

   farray_clear(arr, stride);
   return (farray)arr;
}

static void farray_init(farray *arr, usize capacity, usize stride) {
   // we expect the arr to be uninitialized
   if (!*arr) {
      // allocate memory for the farray structure
      *arr = farray_new(capacity, stride);
   } else {
      // what to do about an farray that's already initialized?
      // for now, we just reallocate the bucket
      if ((*arr)->bucket) {
         Memory.dispose((*arr)->bucket);
      }
      (*arr)->bucket = scope_alloc(stride * capacity, false);
      if (!(*arr)->bucket) {
         // allocation ERRed, handle error as needed
         return;
      }
      (*arr)->end = (char *)((*arr)->bucket) + stride * capacity;
   }
}

static void farray_dispose(farray arr) {
   if (!arr) {
      return; // nothing to dispose
   }

   //  free the bucket and the farray structure itself
   array_free_resources(arr->bucket, arr);
}

static void farray_clear(farray arr, usize stride) {
   array_base_clear((sc_array_base *)arr, stride, farray_element_clear);
}

static int farray_set_at(farray arr, usize index, usize stride, object value) {
   return array_base_set_element((sc_array_base *)arr, stride, index, value, farray_element_copy);
}

static int farray_get_at(farray arr, usize index, usize stride, object out_value) {
   return array_base_get_element((sc_array_base *)arr, stride, index, out_value, farray_element_copy);
}

static int farray_remove_at(farray arr, usize index, usize stride) {
   return array_base_remove_element((sc_array_base *)arr, stride, index, farray_element_clear);
}

#if 1 // Region: Internal utility functions
static int farray_capacity(farray arr, usize stride) {
   return array_base_capacity((sc_array_base *)arr, stride);
}

// compact the array by shifting non-zero elements to the front
usize farray_compact(farray arr, usize stride) {
   return array_base_compact((sc_array_base *)arr, stride, farray_element_is_empty,
                             farray_element_copy, farray_element_clear);
}
#endif

#if 1 // Region: Collection interface functions
// create a non-owning collection view of the farray
static collection farray_as_collection(farray arr, usize stride) {
   if (!arr) {
      return NULL;
   }

   usize length = FArray.capacity(arr, stride);
   return Collections.create_view(arr, stride, length, false);
}

// create an owning collection copy of the farray
static collection farray_to_collection(farray arr, usize stride) {
   if (!arr) {
      return NULL;
   }

   usize capacity = FArray.capacity(arr, stride);
   collection coll = collection_new(capacity, stride);
   if (!coll) {
      return NULL;
   }

   // Treat as farray for storage
   coll->array.handle[0] = 'F';

   // Copy data
   void *src = arr->bucket;
   collection_set_data(coll, src, capacity);

   return coll;
}
#endif

//  public interface implementation
const sc_farray_i FArray = {
    .new = farray_new,
    .init = farray_init,
    .dispose = farray_dispose,
    .capacity = farray_capacity,
    .clear = farray_clear,
    .set = farray_set_at,
    .get = farray_get_at,
    .remove = farray_remove_at,
    .as_collection = farray_as_collection,
    .to_collection = farray_to_collection,
};