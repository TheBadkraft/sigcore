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
 * File: slotarray.c
 * Description: Source file for SigmaCore slotarray definitions and interfaces
 *
 * SlotArray:  A collection structure derived from Array that allows
 *             dynamic resizing, element insertion, removal, and retrieval by index,
 *             with support for "slots" that can be reused after removal. SlotArray
 *             does not compact the underlying array on removal, preserving indices
 *             for existing elements, reusing freed slots for new elements. Compaction
 *             can be performed via a dedicated function if desired.
 */
#include "sigcore/slotarray.h"
#include "sigcore/internal/collections.h"
#include "sigcore/memory.h"
#include <stdlib.h>
#include <string.h>

//  declare the SlotArray struct: with anonymous struct internals
struct sc_slotarray {
   struct {
      void *buffer;
      void *end;
   } array;
   usize stride;
};

// forward declaration of internal functions
static usize find_next_empty_slot(slotarray sa);

// create new slotarray with specified initial capacity
static slotarray slotarray_new(usize capacity) {
   //  allocate memory for the slotarray structure
   slotarray sa = Memory.alloc(sizeof(struct sc_slotarray));
   if (!sa) {
      return NULL; // allocation ERRed
   }
   // Initialize the buffer with the specified capacity
   usize total_size = capacity * sizeof(addr);
   sa->array.buffer = Memory.alloc(total_size);
   if (!sa->array.buffer) {
      Memory.free(sa);
      return NULL; // allocation ERRed
   }
   sa->array.end = (char *)sa->array.buffer + total_size;
   sa->stride = sizeof(addr);
   // initialize all to ADDR_EMPTY
   for (usize i = 0; i < capacity; ++i) {
      addr *ptr = (addr *)((char *)sa->array.buffer + i * sa->stride);
      *ptr = ADDR_EMPTY;
   }
   return sa;
}
// dispose of the slotarray and free its resources
static void slotarray_dispose(slotarray sa) {
   if (!sa) {
      return; // nothing to dispose
   }
   Memory.free(sa->array.buffer);
   Memory.free(sa);
}
// add a value to the slotarray, reusing empty slots if available
static int slotarray_add(slotarray sa, object value) {
   if (!sa) {
      return ERR; // invalid slotarray
   }
   // try to find an empty slot
   usize next_slot = find_next_empty_slot(sa);
   if (next_slot != (usize)-1) {
      // found an empty slot, set the value there
      addr *ptr = (addr *)((char *)sa->array.buffer + next_slot * sa->stride);
      *ptr = (addr)value;
      return (int)next_slot; // return the index where value was added
   } else {
      // no empty slot found, need to grow the array
      usize current_capacity = ((char *)sa->array.end - (char *)sa->array.buffer) / sa->stride;
      usize new_capacity = current_capacity * 2; // double the capacity
      usize new_total_size = new_capacity * sa->stride;
      void *new_buffer = Memory.alloc(new_total_size);
      if (!new_buffer) {
         return ERR; // allocation ERRed
      }
      memcpy(new_buffer, sa->array.buffer, current_capacity * sa->stride);
      Memory.free(sa->array.buffer);
      sa->array.buffer = new_buffer;
      sa->array.end = (char *)new_buffer + new_total_size;
      // initialize new slots to ADDR_EMPTY
      for (usize i = current_capacity; i < new_capacity; ++i) {
         addr *ptr = (addr *)((char *)sa->array.buffer + i * sa->stride);
         *ptr = ADDR_EMPTY;
      }
      // add the new value at the next available slot
      addr *ptr = (addr *)((char *)sa->array.buffer + current_capacity * sa->stride);
      *ptr = (addr)value;
      return (int)current_capacity; // return the index where value was added
   }
}

// find the next empty slot in the slotarray
static usize find_next_empty_slot(slotarray sa) {
   if (!sa) {
      return (usize)-1; // invalid slotarray
   }
   usize cap = ((char *)sa->array.end - (char *)sa->array.buffer) / sa->stride;
   for (usize i = 0; i < cap; ++i) {
      addr *entry = (addr *)((char *)sa->array.buffer + i * sa->stride);
      if (*entry == ADDR_EMPTY) {
         return i; // found empty slot
      }
   }
   return (usize)-1; // no empty slot found
}
// get the value at the specified index in the slotarray
static int slotarray_get_at(slotarray sa, usize index, object *out_value) {
   if (!sa || !out_value) {
      return ERR; // invalid parameters
   }
   usize cap = ((char *)sa->array.end - (char *)sa->array.buffer) / sa->stride;
   if (index >= cap) {
      return ERR; // index out of bounds
   }
   addr *ptr = (addr *)((char *)sa->array.buffer + index * sa->stride);
   if (*ptr == ADDR_EMPTY) {
      return ERR; // slot is empty
   }
   *out_value = (object)*ptr;
   return OK;
}
// remove the element at the specified index from the slotarray
static int slotarray_remove_at(slotarray sa, usize index) {
   if (!sa) {
      return ERR; // invalid slotarray
   }
   usize cap = ((char *)sa->array.end - (char *)sa->array.buffer) / sa->stride;
   if (index >= cap) {
      return ERR; // index out of bounds
   }
   // set the slot to ADDR_EMPTY to mark it as empty
   addr *ptr = (addr *)((char *)sa->array.buffer + index * sa->stride);
   *ptr = ADDR_EMPTY;
   return OK;
}
// check if a slot is empty
static bool slotarray_is_empty_slot(slotarray sa, usize index) {
   if (!sa) {
      return true; // invalid slotarray, consider empty
   }
   usize cap = ((char *)sa->array.end - (char *)sa->array.buffer) / sa->stride;
   if (index >= cap) {
      return true; // out of bounds, consider empty
   }
   addr *ptr = (addr *)((char *)sa->array.buffer + index * sa->stride);
   return *ptr == ADDR_EMPTY;
}
// get the capacity of the slotarray
static usize slotarray_capacity(slotarray sa) {
   if (!sa) {
      return 0; // invalid slotarray
   }
   return ((char *)sa->array.end - (char *)sa->array.buffer) / sa->stride;
}
// clear all slots in the slotarray
static void slotarray_clear(slotarray sa) {
   if (!sa) {
      return; // invalid slotarray
   }
   usize cap = ((char *)sa->array.end - (char *)sa->array.buffer) / sa->stride;
   for (usize i = 0; i < cap; ++i) {
      addr *ptr = (addr *)((char *)sa->array.buffer + i * sa->stride);
      *ptr = ADDR_EMPTY;
   }
}

// create a slotarray from a parray
static slotarray slotarray_from_pointer_array(parray arr) {
   if (!arr) {
      return NULL;
   }
   usize cap = PArray.capacity(arr);
   slotarray sa = SlotArray.new(cap);
   if (!sa) {
      return NULL;
   }
   for (usize i = 0; i < cap; i++) {
      addr value;
      if (PArray.get(arr, i, &value) == OK && value != ADDR_EMPTY) {
         SlotArray.add(sa, (object)value);
      }
   }
   return sa;
}

// create a slotarray from a farray
static slotarray slotarray_from_value_array(farray arr, usize stride) {
   if (!arr) {
      return NULL;
   }
   usize cap = FArray.capacity(arr, stride);
   slotarray sa = SlotArray.new(cap);
   if (!sa) {
      return NULL;
   }
   for (usize i = 0; i < cap; i++) {
      void *value = malloc(stride);
      if (!value) {
         SlotArray.dispose(sa);
         return NULL;
      }
      if (FArray.get(arr, i, stride, value) == OK) {
         SlotArray.add(sa, value);
      } else {
         free(value);
      }
   }
   return sa;
}

// public interface implementation
const sc_slotarray_i SlotArray = {
    .new = slotarray_new,
    .dispose = slotarray_dispose,
    .add = slotarray_add,
    .get_at = slotarray_get_at,
    .remove_at = slotarray_remove_at,
    .from_pointer_array = slotarray_from_pointer_array,
    .from_value_array = slotarray_from_value_array,
    .is_empty_slot = slotarray_is_empty_slot,
    .capacity = slotarray_capacity,
    .clear = slotarray_clear,
};