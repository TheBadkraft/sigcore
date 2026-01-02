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
 * File: array.c
 * Description: Source file for SigmaCore array definitions and interfaces
 *
 * Array: The core collection structure used to unify all collection types
 *        within SigmaCore. List, SlotArray, and Map are all derived from Array
 *        and share common functionality defined here. This means that the
 *        Iterator mechanism can be uniformly applied across all collection types.
 */
#include "sigcore/parray.h"
#include "internal/array_base.h"
#include "internal/arrays.h"
#include "internal/collections.h"
#include "internal/memory_internal.h"
#include "sigcore/collections.h"
#include "sigcore/memory.h"
#include <string.h>

//  declare the PointerArray struct: the collection with attitude
struct sc_pointer_array {
   char handle[2]; // {'P', '\0'} - type identifier
   addr *bucket;   // pointer to first element (array of addr)
   addr end;       // one past allocated memory (as raw addr)
};

#if 1 // Region: Forward declarations
// API functions
static parray array_new(usize);
static void array_init(parray *, usize);
static void array_dispose(parray);
static int array_capacity(parray);
static void array_clear(parray);
static int array_set_at(parray, usize, addr);
static int array_get_at(parray, usize, addr *);
static int array_remove_at(parray, usize);

// Collection interface functions
static collection parray_as_collection(parray arr);
static collection parray_to_collection(parray arr);
#endif

// API function implementations
static parray array_new(usize capacity) {
   void *bucket;
   char *end;

   struct sc_pointer_array *arr = array_alloc_struct_with_bucket(
       sizeof(struct sc_pointer_array), 'P', sizeof(addr), capacity, &bucket, &end);

   if (!arr) {
      return NULL;
   }

   arr->bucket = (addr *)bucket;
   arr->end = (addr)end;

   PArray.clear((parray)arr);
   return (parray)arr;
}

static void array_init(parray *arr, usize capacity) {
   // we expect the arr to be uninitialized
   if (!*arr) {
      // allocate memory for the array structure
      *arr = array_new(capacity);
   } else {
      // what to do about an array that's already initialized?
      // for now, we just reallocate the bucket
      if ((*arr)->bucket) {
         Memory.dispose((*arr)->bucket);
      }
      (*arr)->bucket = scope_alloc(sizeof(addr) * capacity, false);
      if (!(*arr)->bucket) {
         // allocation ERRed, handle error as needed
         return;
      }
      (*arr)->end = (addr)((*arr)->bucket + capacity);
   }
}

static void array_dispose(parray arr) {
   if (!arr) {
      return; // nothing to dispose
   }

   //  free the bucket and the array structure itself
   array_free_resources(arr->bucket, arr);
}

static int array_capacity(parray arr) {
   return array_base_capacity((sc_array_base *)arr, sizeof(addr));
}

static void array_clear(parray arr) {
   array_base_clear((sc_array_base *)arr, sizeof(addr), parray_element_clear);
}

static int array_set_at(parray arr, usize index, addr value) {
   return array_base_set_element((sc_array_base *)arr, sizeof(addr), index, &value, parray_element_copy);
}

static int array_get_at(parray arr, usize index, addr *out_value) {
   return array_base_get_element((sc_array_base *)arr, sizeof(addr), index, out_value, parray_element_copy);
}

static int array_remove_at(parray arr, usize index) {
   return array_base_remove_element((sc_array_base *)arr, sizeof(addr), index, parray_element_clear);
}

#if 1 // Region: Internal utility functions
// compact the array by shifting non-empty elements to the front
usize parray_compact(parray arr) {
   return array_base_compact((sc_array_base *)arr, sizeof(addr), parray_element_is_empty,
                             parray_element_copy, parray_element_clear);
}

// Internal functions for bucket access
addr array_get_bucket_start(parray arr) {
   if (!arr)
      return (addr)0;
   return (addr)arr->bucket;
}

addr array_get_bucket_end(parray arr) {
   if (!arr)
      return (addr)0;
   return arr->end;
}

addr *array_get_bucket(parray arr) {
   if (!arr)
      return NULL;
   return arr->bucket;
}
#endif

#if 1 // Region: Collection interface functions
// create a non-owning collection view of the parray
static collection parray_as_collection(parray arr) {
   if (!arr) {
      return NULL;
   }

   usize length = PArray.capacity(arr);
   return Collections.create_view(arr, sizeof(addr), length, false);
}

// create an owning collection copy of the parray
static collection parray_to_collection(parray arr) {
   if (!arr) {
      return NULL;
   }

   usize capacity = PArray.capacity(arr);
   collection coll = collection_new(capacity, sizeof(addr));
   if (!coll) {
      return NULL;
   }

   // Copy data
   collection_set_data(coll, arr->bucket, capacity);

   return coll;
}
#endif

//  public interface implementation
const sc_parray_i PArray = {
    .new = array_new,
    .init = array_init,
    .dispose = array_dispose,
    .capacity = array_capacity,
    .clear = array_clear,
    .set = array_set_at,
    .get = array_get_at,
    .remove = array_remove_at,
    .as_collection = parray_as_collection,
    .to_collection = parray_to_collection,
};
