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
 * File: array_base.h
 * Description: Unified base array structure and common operations
 */
#pragma once

#include "sigcore/types.h"

// Unified array structure - both farray and parray can be cast to this
typedef struct sc_array_base {
   char handle[2]; // {'F', '\0'} for farray, {'P', '\0'} for parray
   void *bucket;   // pointer to first element (raw bytes or addr*)
   void *end;      // one past allocated memory
} sc_array_base;

// Common array operations that work on the unified structure
int array_base_capacity(const sc_array_base *arr, usize element_size);
bool array_base_is_valid_index(const sc_array_base *arr, usize element_size, usize index);
void *array_base_get_element_ptr(const sc_array_base *arr, usize element_size, usize index);

// Type-specific operations
typedef bool (*array_element_empty_fn)(const void *element, usize element_size);
typedef void (*array_element_clear_fn)(void *element, usize element_size);
typedef void (*array_element_copy_fn)(void *dest, const void *src, usize element_size);

// Generic array operations using callbacks
int array_base_set_element(sc_array_base *arr, usize element_size, usize index,
                           const void *value, array_element_copy_fn copy_fn);
int array_base_get_element(sc_array_base *arr, usize element_size, usize index,
                           void *out_value, array_element_copy_fn copy_fn);
int array_base_remove_element(sc_array_base *arr, usize element_size, usize index,
                              array_element_clear_fn clear_fn);
void array_base_clear(sc_array_base *arr, usize element_size, array_element_clear_fn clear_fn);
usize array_base_compact(sc_array_base *arr, usize element_size,
                         array_element_empty_fn is_empty_fn, array_element_copy_fn copy_fn,
                         array_element_clear_fn clear_fn);

// Type-specific callback implementations
// Flex array callbacks (value semantics)
bool farray_element_is_empty(const void *element, usize element_size);
void farray_element_clear(void *element, usize element_size);
void farray_element_copy(void *dest, const void *src, usize element_size);

// Pointer array callbacks (reference semantics)
bool parray_element_is_empty(const void *element, usize element_size);
void parray_element_clear(void *element, usize element_size);
void parray_element_copy(void *dest, const void *src, usize element_size);