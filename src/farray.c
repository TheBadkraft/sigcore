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
#include "sigcore/internal/arrays.h"
#include "sigcore/internal/collections.h"
#include "sigcore/memory.h"
#include <string.h>

//  declare the FlexArray struct: the collection with attitude
struct sc_flex_array {
   void *bucket; // pointer to first element (raw bytes)
   void *end;    // one past allocated memory
};

// forward declarations of internal functions
static farray farray_new(usize, usize);
static void farray_init(farray *, usize, usize);
static void farray_dispose(farray);
static int farray_capacity(farray, usize);
static void farray_clear(farray, usize);
static int farray_set_at(farray, usize, usize, object);
static int farray_get_at(farray, usize, usize, object);
static int farray_remove_at(farray, usize, usize);

// create a new farray with the specified initial capacity and stride
static farray farray_new(usize capacity, usize stride) {
   //  allocate memory for the farray structure
   struct sc_flex_array *arr = Memory.alloc(sizeof(struct sc_flex_array));
   if (!arr) {
      return NULL; // allocation ERRed
   }

   //  allocate memory for the bucket
   arr->bucket = array_alloc_bucket(stride, capacity);
   if (!arr->bucket) {
      Memory.free(arr);
      return NULL; // allocation ERRed
   }

   arr->end = (char *)arr->bucket + stride * capacity; // set end to the allocated size
   farray_clear(arr, stride);

   return (farray)arr;
}
// initialize farray with the specified capacity and stride
static void farray_init(farray *arr, usize capacity, usize stride) {
   // we expect the arr to be uninitialized
   if (!*arr) {
      // allocate memory for the farray structure
      *arr = farray_new(capacity, stride);
   } else {
      // what to do about an farray that's already initialized?
      // for now, we just reallocate the bucket
      if ((*arr)->bucket) {
         Memory.free((*arr)->bucket);
      }
      (*arr)->bucket = Memory.alloc(stride * capacity);
      if (!(*arr)->bucket) {
         // allocation ERRed, handle error as needed
         return;
      }
      (*arr)->end = (char *)((*arr)->bucket) + stride * capacity;
   }
}
// dispose of the farray and free associated resources
static void farray_dispose(farray arr) {
   if (!arr) {
      return; // nothing to dispose
   }

   //  free the bucket and the farray structure itself
   array_free_resources(arr->bucket, arr);
}
// clear the contents of the farray
static void farray_clear(farray arr, usize stride) {
   if (!arr) {
      return; // invalid farray
   }
   // clear the entire capacity
   size_t num_elements = farray_capacity(arr, stride);
   memset(arr->bucket, 0, num_elements * stride);
}

// set the value at the specified index in the farray
static int farray_set_at(farray arr, usize index, usize stride, object value) {
   if (!arr || !arr->bucket || !value) {
      return ERR; // invalid parameters
   }
   usize cap = farray_capacity(arr, stride);
   if (index >= cap) {
      return ERR; // index out of bounds
   }
   void *dest = (char *)arr->bucket + index * stride;
   memcpy(dest, value, stride);
   return OK; // OK
}
// get the value at the specified index in the farray
static int farray_get_at(farray arr, usize index, usize stride, object out_value) {
   if (!arr || !arr->bucket || !out_value) {
      return ERR; // invalid parameters
   }
   usize cap = farray_capacity(arr, stride);
   if (index >= cap) {
      return ERR; // index out of bounds
   }
   void *src = (char *)arr->bucket + index * stride;
   memcpy(out_value, src, stride);
   return OK; // OK
}
// remove the element at the specified index
static int farray_remove_at(farray arr, usize index, usize stride) {
   /*
      The reason farray does not shift elements left upon removal is to maintain
      consistent performance characteristics and to maintain consistency with
      expectations from derived structures like SlotArray, where shifting elements
      leads to complications with index validity; on the other hand, List does
      shift elements left upon removal to maintain contiguous data for iteration.
    */
   if (!arr || !arr->bucket) {
      return ERR; // invalid farray
   }
   usize cap = farray_capacity(arr, stride);
   if (index >= cap) {
      return ERR; // index out of bounds
   }
   // we do not shift elements, just set to zero
   void *dest = (char *)arr->bucket + index * stride;
   memset(dest, 0, stride);

   return OK; // OK
}

// compact the array by shifting non-zero elements to the front
usize farray_compact(farray arr, usize stride) {
   if (!arr) {
      return 0;
   }

   usize capacity = FArray.capacity(arr, stride);
   usize write_index = 0;

   for (usize read_index = 0; read_index < capacity; ++read_index) {
      // check if element is all zeros (empty)
      void *element = (char *)arr->bucket + read_index * stride;
      bool is_empty = true;
      for (usize i = 0; i < stride; ++i) {
         if (((char *)element)[i] != 0) {
            is_empty = false;
            break;
         }
      }

      if (!is_empty) {
         if (write_index != read_index) {
            void *src = (char *)arr->bucket + read_index * stride;
            void *dest = (char *)arr->bucket + write_index * stride;
            memcpy(dest, src, stride);
            memset(src, 0, stride);
         }
         ++write_index;
      }
   }

   return write_index;
}

// Internal functions
static int farray_capacity(farray arr, usize stride) {
   if (!arr || !arr->bucket) {
      return 0; // invalid farray
   }
   return (int)(((char *)arr->end - (char *)arr->bucket) / stride);
}

// create a non-owning collection view of the farray
static collection farray_as_collection(farray arr, usize stride) {
   if (!arr) {
      return NULL;
   }

   usize length = FArray.capacity(arr, stride);
   return array_create_collection_view(arr->bucket, arr->end, stride, length, false);
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

   // Copy data
   void *src = arr->bucket;
   collection_set_data(coll, src, capacity);

   return coll;
}

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