/*
 * Sigma-Test
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
#include "sigcore/internal/collections.h"
#include "sigcore/memory.h"
#include <string.h>

//  declare the PointerArray struct: the collection with attitude
struct sc_pointer_array {
   addr *bucket; // pointer to first element (array of addr)
   addr end;     // one past allocated memory (as raw addr)
};

// forward declarations of internal functions
static parray array_new(usize);
static void array_dispose(parray);

// create a new array with the specified initial capacity
static parray array_new(usize capacity) {
   //  allocate memory for the array structure
   struct sc_pointer_array *arr = Memory.alloc(sizeof(struct sc_pointer_array));
   if (!arr) {
      return NULL; // allocation ERRed
   }

   //  allocate memory for the bucket
   arr->bucket = Memory.alloc(sizeof(addr) * capacity);
   if (!arr->bucket) {
      Memory.free(arr);
      return NULL; // allocation ERRed
   }

   arr->end = (addr)(arr->bucket + capacity); // set end to the allocated size
   PArray.clear((parray)arr);

   return (parray)arr;
}
// initialize array with the specified capacity
static void array_init(parray *arr, usize capacity) {
   // we expect the arr to be uninitialized
   if (!*arr) {
      // allocate memory for the array structure
      *arr = array_new(capacity);
   } else {
      // what to do about an array that's already initialized?
      // for now, we just reallocate the bucket
      if ((*arr)->bucket) {
         Memory.free((*arr)->bucket);
      }
      (*arr)->bucket = Memory.alloc(sizeof(addr) * capacity);
      if (!(*arr)->bucket) {
         // allocation ERRed, handle error as needed
         return;
      }
      (*arr)->end = (addr)((*arr)->bucket + capacity);
   }
}
// dispose of the array and free associated resources
static void array_dispose(parray arr) {
   if (!arr) {
      return; // nothing to dispose
   }

   //  free the bucket and the array structure itself
   Memory.free(arr->bucket);
   Memory.free(arr);
}

// get the current capacity of the array
static int array_capacity(parray arr) {
   if (!arr || !arr->bucket) {
      return 0; // invalid array
   }
   return (int)((arr->end - (addr)(arr->bucket)) / sizeof(addr));
}
// clear the contents of the array
static void array_clear(parray arr) {
   if (!arr) {
      return; // invalid array
   }
   // clear the entire capacity
   size_t num_elements = array_capacity(arr);
   memset(arr->bucket, 0, num_elements * sizeof(addr));
}

// set the value at the specified index in the array
static int array_set_at(parray arr, usize index, addr value) {
   if (!arr || !arr->bucket) {
      return ERR; // invalid array
   }
   usize cap = array_capacity(arr);
   if (index >= cap) {
      return ERR; // index out of bounds
   }
   arr->bucket[index] = value;
   return OK; // OK
}
// get the value at the specified index in the array
static int array_get_at(parray arr, usize index, addr *out_value) {
   if (!arr || !arr->bucket || !out_value) {
      return ERR; // invalid parameters
   }
   usize cap = array_capacity(arr);
   if (index >= cap) {
      return ERR; // index out of bounds
   }
   *out_value = arr->bucket[index];
   return OK; // OK
}
// remove the element at the specified index
static int array_remove_at(parray arr, usize index) {
   /*
      The reason array does not shift elements left upon removal is to maintain
      consistent performance characteristics and to maintain consitency with
      expectations from derived structures like SlotArray, where shifting elements
      leads to complications with index validity; on the other hand, List does
      shift elements left upon removal to maintain contiguous data for iteration.
    */
   if (!arr || !arr->bucket) {
      return ERR; // invalid array
   }
   usize cap = array_capacity(arr);
   if (index >= cap) {
      return ERR; // index out of bounds
   }
   // we do not shift elements, just set to ADDR_EMPTY
   arr->bucket[index] = ADDR_EMPTY;

   return OK; // OK
}

// Internal function
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
};
