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
 * File: array.h
 * Description: Header file for SigmaCore array definitions and interfaces
 *
 * Array: The core collection structure used to unify all collection types
 *        within SigmaCore. List, SlotArray, and Map are all derived from Array
 *        and share common functionality defined here. This means that the
 *        Iterator mechanism can be uniformly applied across all collection types.
 */
#pragma once

#include "sigcore/types.h"

// forward declaration of the array structure
struct sc_array;
typedef struct sc_array *array;

/* Public interface for array operations                        */
/* ============================================================ */
typedef struct sc_array_i {
   /**
    * @brief Initialize a new array with the specified initial capacity.
    * @param capacity Initial array capacity
    */
   array (*new)(int);
   /**
    * @brief Initialize an array with the specified capacity.
    * @param arr The array to initialize
    * @param capacity Initial array capacity
    */
   void (*init)(array *, int);
   /**
    * @brief Dispose of the array and free associated resources.
    * @param arr The array to dispose of
    */
   void (*dispose)(array);
   /**
    * @brief Get the current capacity of the array.
    * @param arr The array to query
    * @return Current capacity of the array
    */
   int (*capacity)(array);
   /**
    * @brief Clear the contents of the array.
    * @param arr The array to clear
    */
   void (*clear)(array);
   /**
    * @brief Set the value at the specified index in the array.
    * @param arr The array to modify
    * @param index Index at which to set the value
    * @param value Value to set
    * @return 0 on success; otherwise non-zero
    */
   int (*set)(array, int, addr);
   /**
    * @brief Get the value at the specified index in the array.
    * @param arr The array to query
    * @param index Index from which to get the value
    * @param out_value Pointer to store the retrieved value
    * @return 0 on success; otherwise non-zero
    */
   int (*get)(array, int, addr *);
   /**
    * @brief Remove the element at the specified index, shifting remaining elements left.
    * @param arr The array to modify
    * @param index Index of the element to remove
    * @return 0 on success; otherwise non-zero
    */
   int (*remove)(array, int);
} sc_array_i;
extern const sc_array_i Array;