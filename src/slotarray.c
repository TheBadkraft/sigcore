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

//  declare the SlotArray struct: derived from Array
struct sc_slotarray {
   array bucket;
};

// forward declaration of internal functions
static usize find_next_empty_slot(array arr);

// create new slotarray with specified initial capacity
static slotarray slotarray_new(usize capacity) {
   //  allocate memory for the slotarray structure
   slotarray sa = Memory.alloc(sizeof(struct sc_slotarray));
   if (!sa) {
      return NULL; // allocation failed
   }
   // Initialize the underlying array with the specified capacity
   sa->bucket = Array.new(capacity);
   if (!sa->bucket) {
      Memory.free(sa);
      return NULL; // allocation failed
   }
   return sa;
}
// dispose of the slotarray and free its resources
static void slotarray_dispose(slotarray sa) {
   if (!sa) {
      return; // nothing to dispose
   }
   Array.dispose(sa->bucket);
   Memory.free(sa);
}
// add a value to the slotarray, reusing empty slots if available
static int slotarray_add(slotarray sa, object value) {
   if (!sa) {
      return -1; // invalid slotarray
   }
   // try to find an empty slot
   usize next_slot = find_next_empty_slot(sa->bucket);
   if (next_slot != (usize)-1) {
      // found an empty slot, set the value there
      if (Array.set(sa->bucket, next_slot, (addr)value) == 0) {
         return (int)next_slot; // return the index where value was added
      } else {
         return -1; // failed to set value
      }
   } else {
      // no empty slot found, need to grow the array
      usize current_capacity = Array.capacity(sa->bucket);
      usize new_capacity = current_capacity * 2; // double the capacity
      array new_array = Array.new(new_capacity);
      if (!new_array) {
         return -1; // allocation failed
      }
      // copy existing elements to new array
      for (usize i = 0; i < current_capacity; ++i) {
         addr entry;
         if (Array.get(sa->bucket, i, &entry) == 0) {
            Array.set(new_array, i, entry);
         }
      }
      // free the old array and update the slotarray's bucket
      Array.dispose(sa->bucket);
      sa->bucket = new_array;
      // add the new value at the next available slot
      if (Array.set(sa->bucket, current_capacity, (addr)value) == 0) {
         return (int)current_capacity; // return the index where value was added
      } else {
         return -1; // failed to set value
      }
   }
}

// find the next empty slot in the array
static usize find_next_empty_slot(array arr) {
   if (!arr) {
      return (usize)-1; // invalid array
   }
   usize cap = Array.capacity(arr);
   for (usize i = 0; i < cap; ++i) {
      addr entry;
      if (Array.get(arr, i, &entry) == 0 && entry == ADDR_EMPTY) {
         return i; // found empty slot
      }
   }
   return (usize)-1; // no empty slot found
}
// get the value at the specified index in the slotarray
static int slotarray_get_at(slotarray sa, usize index, object *out_value) {
   if (!sa || !out_value) {
      return -1; // invalid parameters
   }
   addr value;
   int result = Array.get(sa->bucket, index, &value);
   if (result == 0) {
      if (value == ADDR_EMPTY) {
         return -1; // slot is empty
      }
      *out_value = (object)value;
      return 0;
   }
   return result;
}
// remove the element at the specified index from the slotarray
static int slotarray_remove_at(slotarray sa, usize index) {
   if (!sa) {
      return -1; // invalid slotarray
   }
   // set the slot to ADDR_EMPTY to mark it as empty
   return Array.set(sa->bucket, index, ADDR_EMPTY);
}
// check if a slot is empty
static bool slotarray_is_empty_slot(slotarray sa, usize index) {
   if (!sa) {
      return true; // invalid slotarray, consider empty
   }
   addr value;
   if (Array.get(sa->bucket, index, &value) == 0) {
      return value == ADDR_EMPTY;
   }
   return true; // invalid index, consider empty
}
// get the capacity of the slotarray
static usize slotarray_capacity(slotarray sa) {
   if (!sa) {
      return 0; // invalid slotarray
   }
   return Array.capacity(sa->bucket);
}
// clear all slots in the slotarray
static void slotarray_clear(slotarray sa) {
   if (!sa) {
      return; // invalid slotarray
   }
   Array.clear(sa->bucket);
}

// public interface implementation
const sc_slotarray_i SlotArray = {
    .new = slotarray_new,
    .dispose = slotarray_dispose,
    .add = slotarray_add,
    .get_at = slotarray_get_at,
    .remove_at = slotarray_remove_at,
    .is_empty_slot = slotarray_is_empty_slot,
    .capacity = slotarray_capacity,
    .clear = slotarray_clear,
};