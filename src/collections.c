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

/* Collection structure                                        */
/* ============================================================ */
struct sc_collection {
   struct {
      void *buffer;
      void *end;
   } array;
   usize stride;
   usize length;
   bool owns_buffer;
};

// create a collection view of array data
collection collection_create_view(void *buffer, void *end, usize stride, usize length, bool owns_buffer) {
   struct sc_collection *coll = Memory.alloc(sizeof(struct sc_collection), false);
   if (!coll) {
      return NULL;
   }

   coll->array.buffer = buffer;
   coll->array.end = end;
   coll->stride = stride;
   coll->length = length;
   coll->owns_buffer = owns_buffer;

   return coll;
}

// set collection data from a buffer
void collection_set_data(collection coll, void *data, usize count) {
   if (!coll || !data) {
      return;
   }

   memcpy(coll->array.buffer, data, count * coll->stride);
   coll->length = count;
}

// collection accessor functions
void *collection_get_buffer(collection coll) {
   return coll ? coll->array.buffer : NULL;
}

inline void *collection_get_end(collection coll) {
   return coll ? coll->array.end : NULL;
}

inline usize collection_get_stride(collection coll) {
   return coll ? coll->stride : 0;
}

inline usize collection_get_length(collection coll) {
   return coll ? coll->length : 0;
}

inline void collection_set_length(collection coll, usize length) {
   if (coll) {
      coll->length = length;
   }
}

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
   struct sc_collection *coll = Memory.alloc(sizeof(struct sc_collection), false);
   if (!coll) {
      return NULL;
   }

   coll->array.buffer = Memory.alloc(stride * capacity, false);
   if (!coll->array.buffer) {
      Memory.dispose(coll);
      return NULL;
   }

   coll->array.end = (char *)coll->array.buffer + stride * capacity;
   coll->stride = stride;
   coll->length = 0;
   coll->owns_buffer = true;
   return coll;
}
// dispose of the collection
void collection_dispose(collection coll) {
   if (!coll) {
      return;
   }

   if (coll->owns_buffer && coll->array.buffer) {
      Memory.dispose(coll->array.buffer);
   }
   Memory.dispose(coll);
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
      return ERR;
   }
   usize current_capacity = ((char *)coll->array.end - (char *)coll->array.buffer) / coll->stride;
   usize new_capacity = current_capacity * 2;
   void *new_buffer = Memory.alloc(coll->stride * new_capacity, false);
   if (!new_buffer) {
      return ERR;
   }

   memcpy(new_buffer, coll->array.buffer, coll->stride * current_capacity);
   Memory.dispose(coll->array.buffer);
   coll->array.buffer = new_buffer;
   coll->array.end = (char *)new_buffer + coll->stride * new_capacity;
   return OK;
}

// add an element to the collection
int collection_add(collection coll, object ptr) {
   if (!coll || !ptr) {
      return ERR;
   }

   usize capacity = ((char *)coll->array.end - (char *)coll->array.buffer) / coll->stride;
   if (coll->length >= capacity) {
      if (collection_grow(coll) != 0) {
         return ERR;
      }
   }

   void *dest = (char *)coll->array.buffer + coll->length * coll->stride;
   memcpy(dest, ptr, coll->stride);
   coll->length++;
   return OK;
}
// remove an element from the collection
int collection_remove(collection coll, object ptr) {
   if (!coll || !ptr) {
      return ERR;
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
         return OK;
      }
   }
   return ERR; // not found
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

/* New Iterator implementation */
struct iterator_s {
   collection coll; /* The collection to iterate over */
   size_t current;  /* Current index */
};

/* Create an iterator for a collection */
iterator collection_create_iterator(collection coll) {
   if (!coll)
      return NULL;
   iterator it = Memory.alloc(sizeof(struct iterator_s), false);
   if (!it)
      return NULL;
   it->coll = coll;
   it->current = 0;
   return it;
}

//  public interface implementation
const sc_collections_i Collections = {
    .add = collection_add,
    .remove = collection_remove,
    .clear = collection_clear,
    .count = collection_get_count,
    .create_iterator = collection_create_iterator,
    .create_view = collection_create_view,
    .dispose = collection_dispose,
};

/* Advances to next item and returns true if there is one */
static bool iter_next(iterator it) {
   if (!it || !it->coll || it->current >= it->coll->length)
      return false;
   it->current++;
   return true;
}

/* Returns the current item, or NULL if none */
static object iter_current(iterator it) {
   if (!it || !it->coll || it->current == 0 || it->current > it->coll->length)
      return NULL;
   return (char *)it->coll->array.buffer + (it->current - 1) * it->coll->stride;
}

/* Resets the iterator to the start */
static void iter_reset(iterator it) {
   if (it)
      it->current = 0;
}

/* Disposes the iterator */
static void iter_dispose(iterator it) {
   if (it)
      Memory.dispose(it);
}

const sc_iterator_i Iterator = {
    .next = iter_next,
    .current = iter_current,
    .reset = iter_reset,
    .dispose = iter_dispose};