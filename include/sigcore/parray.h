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
 * File: parray.h
 * Description: Header file for SigmaCore pointer-array definitions and interfaces
 *
 * Array: One of the two core collection structures used to abstract and unify all
 *        collection types within SigmaCore. The pointer-array (parray) allows
 *        storing pointers (addresses) to objects, enabling dynamic collections
 *        of arbitrary objects via pointer indirection.
 */
#pragma once

#include "sigcore/collection.h"
#include "sigcore/types.h"

// forward declaration of the collection structure
// struct sc_collection;  // moved to collection.h
// typedef struct sc_collection *collection;  // moved to collection.h

// forward declaration of the array structure
struct sc_pointer_array;
typedef struct sc_pointer_array *parray;

/* Public interface for array operations                        */
/* ============================================================ */
typedef struct sc_parray_i {
   /**
    * @brief Initialize a new array with the specified initial capacity.
    * @param capacity Initial array capacity
    */
   parray (*new)(usize);
   /**
    * @brief Initialize an array with the specified capacity.
    * @param arr The array to initialize
    * @param capacity Initial array capacity
    */
   void (*init)(parray *, usize);
   /**
    * @brief Dispose of the array and free associated resources.
    * @param arr The array to dispose of
    */
   void (*dispose)(parray);
   /**
    * @brief Get the current capacity of the array.
    * @param arr The array to query
    * @return Current capacity of the array
    */
   int (*capacity)(parray);
   /**
    * @brief Clear the contents of the array.
    * @param arr The array to clear
    */
   void (*clear)(parray);
   /**
    * @brief Set the value at the specified index in the array.
    * @param arr The array to modify
    * @param index Index at which to set the value
    * @param value Value to set
    * @return 0 on OK; otherwise non-zero
    */
   int (*set)(parray, usize, addr);
   /**
    * @brief Get the value at the specified index in the array.
    * @param arr The array to query
    * @param index Index from which to get the value
    * @param out_value Pointer to store the retrieved value
    * @return 0 on OK; otherwise non-zero
    */
   int (*get)(parray, usize, addr *);
   /**
    * @brief Remove the element at the specified index, setting it to empty without shifting.
    * @param arr The array to modify
    * @param index Index of the element to remove
    * @return 0 on OK; otherwise non-zero
    */
   int (*remove)(parray, usize);
   /**
    * @brief Create a non-owning collection view of the array.
    * @param arr The array to view
    * @return A collection view, or NULL on failure
    */
   collection (*as_collection)(parray);
   /**
    * @brief Create an owning collection copy of the array.
    * @param arr The array to copy
    * @return A collection copy, or NULL on failure
    */
   collection (*to_collection)(parray);
} sc_parray_i;
extern const sc_parray_i PArray;