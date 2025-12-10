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
 * File: farray.h
 * Description: Header file for SigmaCore flex-array definitions and interfaces
 *
 * Array: One of the two core collection structures used to abstract and unify all
 *        collection types within SigmaCore. The flex-array (farray) allows storing
 *        elements of arbitrary size directly, without the overhead of pointer
 *        indirection. This is useful for small types like integers or structs
 *        where memory efficiency is important.
 */
#pragma once

#include "sigcore/types.h"

// forward declaration of the array structure
struct sc_flex_array;
typedef struct sc_flex_array *farray;

/* Public interface for array operations                        */
/* ============================================================ */
typedef struct sc_farray_i {
   /**
    * @brief Initialize a new array with the specified initial capacity.
    * @param capacity Initial array capacity
    * @param stride Size of each element in the array
    */
   farray (*new)(usize, usize);
   /**
    * @brief Initialize an array with the specified capacity.
    * @param arr The array to initialize
    * @param capacity Initial array capacity
    * @param stride Size of each element in the array
    */
   void (*init)(farray *, usize, usize);
   /**
    * @brief Dispose of the array and free associated resources.
    * @param arr The array to dispose of
    */
   void (*dispose)(farray);
   /**
    * @brief Get the current capacity of the array.
    * @param arr The array to query
    * @param stride Size of each element in the array
    * @return Current capacity of the array
    */
   int (*capacity)(farray, usize);
   /**
    * @brief Clear the contents of the array.
    * @param arr The array to clear
    * @param stride Size of each element in the array
    */
   void (*clear)(farray, usize);
   /**
    * @brief Set the value at the specified index in the array.
    * @param arr The array to modify
    * @param index Index at which to set the value
    * @param stride Size of each element in the array
    * @param value Pointer to the value to set (must be at least stride bytes)
    * @return 0 on OK; otherwise non-zero
    */
   int (*set)(farray, usize, usize, object);
   /**
    * @brief Get the value at the specified index in the array.
    * @param arr The array to query
    * @param index Index from which to get the value
    * @param stride Size of each element in the array
    * @param out_value Pointer to store the retrieved value (must be at least stride bytes)
    * @return 0 on OK; otherwise non-zero
    */
   int (*get)(farray, usize, usize, void *);
   /**
    * @brief Remove the element at the specified index, setting it to empty without shifting.
    * @param arr The array to modify
    * @param index Index of the element to remove
    * @param stride Size of each element in the array
    * @return 0 on OK; otherwise non-zero
    */
   int (*remove)(farray, usize, usize);
} sc_farray_i;
extern const sc_farray_i FArray;

// Internal access functions (for collections)
void *farray_get_bucket(farray arr);
void *farray_get_bucket_end(farray arr);
int farray_capacity(farray arr, usize stride);