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
#include "sigcore/array.h"
#include "sigcore/memory.h"
#include <string.h>

//  now declare the Array struct: the collection with attitude
struct sc_array {
   addr *bucket; // pointer to first element (array of addr)
   addr end;     // one past allocated memory (as raw addr)
};

// forward declarations of internal functions
static array array_new(int);
static void array_dispose(array);

// create a new array with the specified initial capacity
static array array_new(int capacity) {
   //  allocate memory for the array structure
   struct sc_array *arr = Memory.alloc(sizeof(struct sc_array));
   if (!arr) {
      return NULL; // allocation failed
   }

   //  allocate memory for the bucket
   arr->bucket = Memory.alloc(sizeof(addr) * capacity);
   if (!arr->bucket) {
      Memory.free(arr);
      return NULL; // allocation failed
   }

   arr->end = (addr)(arr->bucket + capacity); // set end to the allocated size
   Array.clear((array)arr);

   return (array)arr;
}
// initialize array with the specified capacity
static void array_init(array *arr, int capacity) {
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
         // allocation failed, handle error as needed
         return;
      }
      (*arr)->end = (addr)((*arr)->bucket + capacity);
   }
}
// dispose of the array and free associated resources
static void array_dispose(array arr) {
   if (!arr) {
      return; // nothing to dispose
   }

   //  free the bucket and the array structure itself
   Memory.free(arr->bucket);
   Memory.free(arr);
}
static int array_capacity(array arr) {
   if (!arr || !arr->bucket) {
      return 0; // invalid array
   }
   return (int)((arr->end - (addr)(arr->bucket)) / sizeof(addr));
}
static void array_clear(array arr) {
   if (!arr) {
      return; // invalid array
   }
   // clear the entire capacity
   size_t num_elements = array_capacity(arr);
   memset(arr->bucket, 0, num_elements * sizeof(addr));
}
// handle value set and get functions
static int array_set_at(array arr, int index, addr value) {
   if (!arr || !arr->bucket) {
      return -1; // invalid array
   }
   int cap = array_capacity(arr);
   if (index < 0 || index >= cap) {
      return -1; // index out of bounds
   }
   arr->bucket[index] = value;
   return 0; // success
}
static int array_get_at(array arr, int index, addr *out_value) {
   if (!arr || !arr->bucket || !out_value) {
      return -1; // invalid parameters
   }
   int cap = array_capacity(arr);
   if (index < 0 || index >= cap) {
      return -1; // index out of bounds
   }
   *out_value = arr->bucket[index];
   return 0; // success
}
static int array_remove_at(array arr, int index) {
   if (!arr || !arr->bucket) {
      return -1; // invalid array
   }
   int cap = array_capacity(arr);
   if (index < 0 || index >= cap) {
      return -1; // index out of bounds
   }
   // Shift elements left to fill the gap
   for (int i = index; i < cap - 1; i++) {
      arr->bucket[i] = arr->bucket[i + 1];
   }
   // Clear the last element
   arr->bucket[cap - 1] = (addr)0;
   return 0; // success
}

//  public interface implementation
const sc_array_i Array = {
    .new = array_new,
    .init = array_init,
    .dispose = array_dispose,
    .capacity = array_capacity,
    .clear = array_clear,
    .set = array_set_at,
    .get = array_get_at,
    .remove = array_remove_at,
};
