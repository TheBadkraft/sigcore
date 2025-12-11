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
 * File: slotarray.h
 * Description: Header file for SigmaCore slotarray definitions and interfaces
 *
 * SlotArray:  A collection structure derived from Array that allows
 *             dynamic resizing, element insertion, removal, and retrieval by index,
 *             with support for "slots" that can be reused after removal. SlotArray
 *             does not compact the underlying array on removal, preserving indices
 *             for existing elements, reusing freed slots for new elements. Compaction
 *             can be performed via a dedicated function if desired.
 */
#pragma once

#include "sigcore/farray.h"
#include "sigcore/parray.h"
struct sc_slotarray;
typedef struct sc_slotarray *slotarray;

/* Public interface for slotarray operations                    */
/* ============================================================ */
typedef struct sc_slotarray_i {
   /**
    * @brief Create a new SlotArray with the specified initial capacity.
    * @param capacity The initial number of slots to allocate.
    * @return A pointer to the newly created SlotArray, or NULL on ERRure.
    */
   slotarray (*new)(usize);
   /**
    * @brief Dispose of the SlotArray and free its resources.
    * @param sa The SlotArray to dispose.
    */
   void (*dispose)(slotarray);
   /**
    * @brief Add a value to the SlotArray, reusing empty slots if available.
    * @param sa The SlotArray to add the value to.
    * @param value The value to add.
    * @return The index (handle) where the value was added; otherwise -1.
    */
   int (*add)(slotarray, object);
   /**
    * @brief Retrieve the value at the specified index (handle) in the SlotArray.
    * @param sa The SlotArray to retrieve the value from.
    * @param index The index (handle) of the value to retrieve.
    * @param out_value Pointer to store the retrieved value.
    * @return 0 on OK; otherwise non-zero
    */
   int (*get_at)(slotarray, usize, object *);
   /**
    * @brief Remove the element at the specified index (handle) from the SlotArray.
    * @param sa The SlotArray to remove the element from.
    * @param index The index (handle) of the element to remove.
    * @return 0 on OK; otherwise non-zero
    */
   int (*remove_at)(slotarray, usize);

   /**
    * @brief Create a SlotArray from a pointer array, copying all non-empty elements.
    * @param arr The pointer array to copy from.
    * @return A new SlotArray with the elements, or NULL on failure.
    */
   slotarray (*from_pointer_array)(parray);

   /**
    * @brief Create a SlotArray from a value array, copying all elements.
    * @param arr The value array to copy from.
    * @param stride The size of each element.
    * @return A new SlotArray with the elements, or NULL on failure.
    */
   slotarray (*from_value_array)(farray, usize);

   // Introspection
   bool (*is_empty_slot)(slotarray, usize); // Check if slot is empty
   usize (*capacity)(slotarray);            // Total slots
   void (*clear)(slotarray);                // Reset all
} sc_slotarray_i;
extern const sc_slotarray_i SlotArray;