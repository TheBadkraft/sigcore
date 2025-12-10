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
 * File: list.c
 * Description: Source file for SigmaCore list definitions and interfaces
 *
 * List:    An ordered collection structure derived from Array that allows
 *          dynamic resizing, element insertion, removal, and retrieval by index,
 *          or by appending to the end.
 */
#include "sigcore/list.h"
#include "sigcore/collections.h"
#include "sigcore/internal/collections.h"
#include "sigcore/memory.h"
#include <string.h>

//  declare the List struct: derived from Collection
struct sc_list {
   collection coll; // underlying collection
};

//  create new list with specified initial capacity
static list list_new(usize capacity) {
   //  allocate memory for the list structure
   struct sc_list *lst = Memory.alloc(sizeof(struct sc_list));
   if (!lst) {
      return NULL; // allocation ERRed
   }

   //  create the underlying collection with stride for addr
   lst->coll = collection_new(capacity, sizeof(addr));
   if (!lst->coll) {
      Memory.free(lst);
      return NULL; // allocation ERRed
   }

   return lst;
}
//  dispose of the list
static void list_dispose(list lst) {
   if (!lst) {
      return; // nothing to dispose
   }

   //  dispose of the underlying collection
   collection_dispose(lst->coll);

   //  free the list structure itself
   Memory.free(lst);
}

//  get the current capacity of the list
static usize list_capacity(list lst) {
   if (!lst) {
      return 0; // invalid list
   }
   return ((char *)lst->coll->array.end - (char *)lst->coll->array.buffer) / lst->coll->stride;
}
//  get the current size of the list
static usize list_size(list lst) {
   if (!lst) {
      return 0; // invalid list
   }
   return lst->coll->length;
}
//  append a value to the end of the list
static int list_append(list lst, object value) {
   if (!lst || !value) {
      return ERR; // invalid parameters
   }
   return collection_add(lst->coll, &value);
}
//  get the value at the specified index in the list
static int list_get_at(list lst, usize index, object *out_value) {
   if (!lst || !out_value) {
      return ERR; // invalid parameters
   }
   if (index >= lst->coll->length) {
      return ERR; // index out of bounds
   }
   void *src = (char *)lst->coll->array.buffer + index * lst->coll->stride;
   memcpy(out_value, src, lst->coll->stride);
   return OK;
}
//  remove the element at the specified index from the list
static int list_remove_at(list lst, usize index) {
   if (!lst) {
      return ERR; // invalid list
   }
   usize size = lst->coll->length;
   if (index >= size) {
      return ERR; // index out of bounds
   }
   // Shift left from index to end-1
   for (usize i = index; i < size - 1; ++i) {
      void *src = (char *)lst->coll->array.buffer + (i + 1) * lst->coll->stride;
      void *dst = (char *)lst->coll->array.buffer + i * lst->coll->stride;
      memcpy(dst, src, lst->coll->stride);
   }
   // Zero the last
   object zero = NULL;
   void *last = (char *)lst->coll->array.buffer + (size - 1) * lst->coll->stride;
   memcpy(last, &zero, lst->coll->stride);
   lst->coll->length--;
   return OK;
}
// set the value at the specified index in the list
static int list_set_at(list lst, usize index, object value) {
   if (!lst) {
      return ERR; // invalid parameters
   }
   usize size = lst->coll->length;
   if (index >= size) {
      return ERR; // index out of bounds
   }
   void *dst = (char *)lst->coll->array.buffer + index * lst->coll->stride;
   memcpy(dst, &value, lst->coll->stride);
   return OK;
}
static int list_insert_at(list lst, usize index, object value) {
   // NULLs are allowed ... not my call, that's on the user
   if (!lst) {
      return ERR; // invalid parameters
   }
   usize size = lst->coll->length;
   if (index > size) {
      return ERR; // index out of bounds
   }
   usize capacity = ((char *)lst->coll->array.end - (char *)lst->coll->array.buffer) / lst->coll->stride;
   if (size >= capacity) {
      if (collection_grow(lst->coll) != 0) {
         return ERR; // growth ERRed
      }
   }
   // Shift right from index to end
   for (usize i = size; i > index; --i) {
      void *src = (char *)lst->coll->array.buffer + (i - 1) * lst->coll->stride;
      void *dst = (char *)lst->coll->array.buffer + i * lst->coll->stride;
      memcpy(dst, src, lst->coll->stride);
   }
   // Insert the new value
   void *ins = (char *)lst->coll->array.buffer + index * lst->coll->stride;
   memcpy(ins, &value, lst->coll->stride);
   lst->coll->length++;
   return OK;
}
// prepend a value to the start of the list
static int list_prepend(list lst, object value) {
   if (!lst || !value) {
      return ERR; // invalid parameters
   }
   return list_insert_at(lst, 0, value);
}
// clear the contents of the list
static void list_clear(list lst) {
   if (!lst) {
      return; // invalid list
   }
   collection_clear(lst->coll);
}

//  public interface implementation
const sc_list_i List = {
    .new = list_new,
    .dispose = list_dispose,
    .capacity = list_capacity,
    .size = list_size,
    .append = list_append,
    .get = list_get_at,
    .remove = list_remove_at,
    .set = list_set_at,
    .insert = list_insert_at,
    .prepend = list_prepend,
    .clear = list_clear,
};
