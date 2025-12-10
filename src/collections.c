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
 * File: collection.c
 * Description: Source file for SigmaCore collection definitions and interfaces
 *
 * Collection: A generic collection wrapper that provides a unified interface
 *             for arrays (parray/farray), with stride-aware operations.
 */
#include "sigcore/collections.h"
#include "sigcore/internal/collections.h"
#include "sigcore/memory.h"
#include <string.h>

// forward declarations of internal functions
collection collection_new(usize capacity, usize stride);
void collection_dispose(collection coll);
usize collection_count(collection coll);
int collection_grow(collection coll);
int collection_add(collection coll, object ptr);
int collection_remove(collection coll, object ptr);
void collection_clear(collection coll);
usize collection_get_count(collection coll);
collection collection_as_collection(farray arr, usize stride);
collection collection_to_collection(farray arr, usize stride);

// create a new collection with the specified capacity and stride
collection collection_new(usize capacity, usize stride) {
   struct sc_collection *coll = Memory.alloc(sizeof(struct sc_collection));
   if (!coll) {
      return NULL;
   }

   coll->array.buffer = Memory.alloc(stride * capacity);
   if (!coll->array.buffer) {
      Memory.free(coll);
      return NULL;
   }

   coll->array.end = (char *)coll->array.buffer + stride * capacity;
   coll->stride = stride;
   coll->length = 0;
   return coll;
}
// dispose of the collection
void collection_dispose(collection coll) {
   if (!coll) {
      return;
   }

   if (coll->array.buffer) {
      Memory.free(coll->array.buffer);
   }
   Memory.free(coll);
}
// get the count of elements in the collection
usize collection_count(collection coll) {
   if (!coll) {
      return 0;
   }
   return coll->length;
}
// grow the collection
int collection_grow(collection coll) {
   if (!coll) {
      return -1;
   }
   usize current_capacity = ((char *)coll->array.end - (char *)coll->array.buffer) / coll->stride;
   usize new_capacity = current_capacity * 2;
   void *new_buffer = Memory.alloc(coll->stride * new_capacity);
   if (!new_buffer) {
      return -1;
   }

   memcpy(new_buffer, coll->array.buffer, coll->stride * current_capacity);
   Memory.free(coll->array.buffer);
   coll->array.buffer = new_buffer;
   coll->array.end = (char *)new_buffer + coll->stride * new_capacity;
   return 0;
}

// add an element to the collection
int collection_add(collection coll, object ptr) {
   if (!coll || !ptr) {
      return -1;
   }

   usize capacity = ((char *)coll->array.end - (char *)coll->array.buffer) / coll->stride;
   if (coll->length >= capacity) {
      if (collection_grow(coll) != 0) {
         return -1;
      }
   }

   void *dest = (char *)coll->array.buffer + coll->length * coll->stride;
   memcpy(dest, ptr, coll->stride);
   coll->length++;
   return 0;
}
// remove an element from the collection
int collection_remove(collection coll, object ptr) {
   if (!coll || !ptr) {
      return -1;
   }

   for (usize i = 0; i < coll->length; ++i) {
      void *slot = (char *)coll->array.buffer + i * coll->stride;
      if (memcmp(slot, ptr, coll->stride) == 0) {
         // Shift left
         for (usize j = i; j < coll->length - 1; ++j) {
            void *src = (char *)coll->array.buffer + (j + 1) * coll->stride;
            void *dest = (char *)coll->array.buffer + j * coll->stride;
            memcpy(dest, src, coll->stride);
         }
         // Zero the last
         memset((char *)coll->array.buffer + (coll->length - 1) * coll->stride, 0, coll->stride);
         coll->length--;
         return 0;
      }
   }
   return -1; // not found
}
// clear the collection
void collection_clear(collection coll) {
   if (!coll || !coll->array.buffer) {
      return;
   }
   usize capacity = ((char *)coll->array.end - (char *)coll->array.buffer) / coll->stride;
   memset(coll->array.buffer, 0, capacity * coll->stride);
   coll->length = 0;
}
// get count
usize collection_get_count(collection coll) {
   return collection_count(coll);
}
// create a non-owning collection view
collection collection_as_collection(farray arr, usize stride) {
   if (!arr) {
      return NULL;
   }

   struct sc_collection *coll = Memory.alloc(sizeof(struct sc_collection));
   if (!coll) {
      return NULL;
   }

   coll->array.buffer = farray_get_bucket(arr);
   coll->array.end = farray_get_bucket_end(arr);
   coll->stride = stride;
   coll->length = FArray.capacity(arr, stride); // Assume full

   return coll;
}
// create an owning collection copy
collection collection_to_collection(farray arr, usize stride) {
   if (!arr) {
      return NULL;
   }

   usize capacity = FArray.capacity(arr, stride);
   collection coll = collection_new(capacity, stride);
   if (!coll) {
      return NULL;
   }

   // Copy data
   void *src = farray_get_bucket(arr);
   memcpy(coll->array.buffer, src, capacity * stride);
   coll->length = capacity;

   return coll;
}

// compact a parray by shifting non-empty elements to the front
static usize collections_compact(parray arr) {
   if (!arr) {
      return 0;
   }

   usize capacity = PArray.capacity(arr);
   usize write_index = 0;

   for (usize read_index = 0; read_index < capacity; ++read_index) {
      addr value;
      if (PArray.get(arr, read_index, &value) == 0 && value != ADDR_EMPTY) {
         if (write_index != read_index) {
            PArray.set(arr, write_index, value);
            PArray.set(arr, read_index, ADDR_EMPTY);
         }
         ++write_index;
      }
   }

   return write_index;
}

//  public interface implementation
const sc_collections_i Collections = {
    .compact = collections_compact,
    .add = collection_add,
    .remove = collection_remove,
    .clear = collection_clear,
    .count = collection_get_count,
    .as_collection = collection_as_collection,
    .to_collection = collection_to_collection,
};