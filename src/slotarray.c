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
 * File: slotarray.c
 * Description: Source file for SigmaCore slotarray definitions and interfaces
 *
 * SlotArray:  A collection structure derived from Array that allows
 *             element insertion, removal, and retrieval by index,
 *             with support for "slots" that can be reused after removal. SlotArray
 *             does not compact the underlying array on removal, preserving indices
 *             for existing elements, reusing freed slots for new elements. Compaction
 *             can be performed via a dedicated function if desired.
 */
#include "sigcore/slotarray.h"
#include "internal/collections.h"
#include "internal/memory_internal.h"
#include "sigcore/memory.h"
#include "sigcore/parray.h"
#include <stdlib.h>
#include <string.h>

//  declare the SlotArray struct: uses parray internally
struct sc_slotarray {
   parray array;    // underlying parray for storage
   usize next_slot; // next slot to check for reuse
};

// forward declaration of internal functions
// (none needed)

// create new slotarray with specified initial capacity
static slotarray slotarray_new(usize capacity) {
   //  allocate memory for the slotarray structure
   slotarray sa = scope_alloc(sizeof(struct sc_slotarray), false);
   if (!sa) {
      return NULL; // allocation ERRed
   }

   // create underlying parray
   sa->array = PArray.new(capacity);
   if (!sa->array) {
      Memory.dispose(sa);
      return NULL;
   }

   sa->next_slot = 0;
   return sa;
}
// dispose of the slotarray and free its resources
static void slotarray_dispose(slotarray sa) {
   if (!sa) {
      return; // nothing to dispose
   }
   PArray.dispose(sa->array);
   Memory.dispose(sa);
}
// add a value to the slotarray, reusing empty slots if available
static int slotarray_add(slotarray sa, object value) {
   if (!sa) {
      return ERR; // invalid slotarray
   }

   // try to find an empty slot starting from next_slot
   usize capacity = PArray.capacity(sa->array);
   for (usize i = 0; i < capacity; ++i) {
      usize slot_index = (sa->next_slot + i) % capacity;
      addr current_value;
      if (PArray.get(sa->array, slot_index, &current_value) == OK && current_value == ADDR_EMPTY) {
         // found an empty slot, set the value there
         PArray.set(sa->array, slot_index, (addr)value);
         sa->next_slot = (slot_index + 1) % capacity; // update next_slot for next search
         return (int)slot_index;                      // return the index where value was added
      }
   }

   // no empty slot found, cannot add
   return ERR;
}

// get the value at the specified index in the slotarray
static int slotarray_get_at(slotarray sa, usize index, object *out_value) {
   if (!sa || !out_value) {
      return ERR; // invalid parameters
   }
   addr value;
   if (PArray.get(sa->array, index, &value) != OK) {
      return ERR; // index out of bounds or other error
   }
   if (value == ADDR_EMPTY) {
      return ERR; // slot is empty
   }
   *out_value = (object)value;
   return OK;
}
// remove the element at the specified index from the slotarray
static int slotarray_remove_at(slotarray sa, usize index) {
   if (!sa) {
      return ERR; // invalid slotarray
   }
   // set the slot to ADDR_EMPTY to mark it as empty
   PArray.set(sa->array, index, ADDR_EMPTY);
   return OK;
}
// check if a slot is empty
static bool slotarray_is_empty_slot(slotarray sa, usize index) {
   if (!sa) {
      return true; // invalid slotarray, consider empty
   }
   addr value;
   if (PArray.get(sa->array, index, &value) != OK) {
      return true; // out of bounds or error, consider empty
   }
   return value == ADDR_EMPTY;
}

// get the capacity of the slotarray
static usize slotarray_capacity(slotarray sa) {
   if (!sa) {
      return 0; // invalid slotarray
   }
   return PArray.capacity(sa->array);
}

// clear all slots in the slotarray
static void slotarray_clear(slotarray sa) {
   if (!sa) {
      return; // invalid slotarray
   }
   PArray.clear(sa->array);
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