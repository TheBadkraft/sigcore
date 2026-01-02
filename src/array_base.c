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
 * File: array_base.c
 * Description: Implementation of unified base array operations
 */
#include "internal/array_base.h"
#include "sigcore/types.h"
#include <string.h>

// Get capacity of any array type
int array_base_capacity(const sc_array_base *arr, usize element_size) {
   if (!arr || !arr->bucket) {
      return 0;
   }
   return (int)((arr->end - arr->bucket) / element_size);
}

// Check if index is valid for the array
bool array_base_is_valid_index(const sc_array_base *arr, usize element_size, usize index) {
   return arr && arr->bucket && index < (usize)array_base_capacity(arr, element_size);
}

// Get pointer to element at index
void *array_base_get_element_ptr(const sc_array_base *arr, usize element_size, usize index) {
   if (!array_base_is_valid_index(arr, element_size, index)) {
      return NULL;
   }
   return (char *)arr->bucket + index * element_size;
}

// Generic set operation using callback
int array_base_set_element(sc_array_base *arr, usize element_size, usize index,
                          const void *value, array_element_copy_fn copy_fn) {
   if (!arr || !arr->bucket || !value) {
      return ERR;
   }
   if (!array_base_is_valid_index(arr, element_size, index)) {
      return ERR;
   }

   void *dest = array_base_get_element_ptr(arr, element_size, index);
   copy_fn(dest, value, element_size);
   return OK;
}

// Generic get operation using callback
int array_base_get_element(sc_array_base *arr, usize element_size, usize index,
                          void *out_value, array_element_copy_fn copy_fn) {
   if (!arr || !arr->bucket || !out_value) {
      return ERR;
   }
   if (!array_base_is_valid_index(arr, element_size, index)) {
      return ERR;
   }

   void *src = array_base_get_element_ptr(arr, element_size, index);
   copy_fn(out_value, src, element_size);
   return OK;
}

// Generic remove operation using callback
int array_base_remove_element(sc_array_base *arr, usize element_size, usize index,
                             array_element_clear_fn clear_fn) {
   if (!arr || !arr->bucket) {
      return ERR;
   }
   if (!array_base_is_valid_index(arr, element_size, index)) {
      return ERR;
   }

   void *element = array_base_get_element_ptr(arr, element_size, index);
   clear_fn(element, element_size);
   return OK;
}

// Generic clear operation using callback
void array_base_clear(sc_array_base *arr, usize element_size, array_element_clear_fn clear_fn) {
   if (!arr) {
      return;
   }

   int capacity = array_base_capacity(arr, element_size);
   for (int i = 0; i < capacity; ++i) {
      void *element = array_base_get_element_ptr(arr, element_size, i);
      if (element) {
         clear_fn(element, element_size);
      }
   }
}

// Generic compact operation using callbacks
usize array_base_compact(sc_array_base *arr, usize element_size,
                        array_element_empty_fn is_empty_fn, array_element_copy_fn copy_fn,
                        array_element_clear_fn clear_fn) {
   if (!arr) {
      return 0;
   }

   usize capacity = array_base_capacity(arr, element_size);
   usize write_index = 0;

   for (usize read_index = 0; read_index < capacity; ++read_index) {
      void *element = array_base_get_element_ptr(arr, element_size, read_index);
      if (element && !is_empty_fn(element, element_size)) {
         if (write_index != read_index) {
            void *dest = array_base_get_element_ptr(arr, element_size, write_index);
            void *src = element;
            copy_fn(dest, src, element_size);
            clear_fn(element, element_size);
         }
         ++write_index;
      }
   }

   return write_index;
}

// Flex array callbacks (value semantics - memcpy/memset)
bool farray_element_is_empty(const void *element, usize element_size) {
   const char *bytes = (const char *)element;
   for (usize i = 0; i < element_size; ++i) {
      if (bytes[i] != 0) {
         return false;
      }
   }
   return true;
}

void farray_element_clear(void *element, usize element_size) {
   memset(element, 0, element_size);
}

void farray_element_copy(void *dest, const void *src, usize element_size) {
   memcpy(dest, src, element_size);
}

// Pointer array callbacks (reference semantics - direct assignment)
bool parray_element_is_empty(const void *element, usize element_size) {
   (void)element_size; // Always sizeof(addr) for parray
   const addr *ptr = (const addr *)element;
   return *ptr == ADDR_EMPTY;
}

void parray_element_clear(void *element, usize element_size) {
   (void)element_size; // Always sizeof(addr) for parray
   addr *ptr = (addr *)element;
   *ptr = ADDR_EMPTY;
}

void parray_element_copy(void *dest, const void *src, usize element_size) {
   (void)element_size; // Always sizeof(addr) for parray
   const addr *src_ptr = (const addr *)src;
   addr *dest_ptr = (addr *)dest;
   *dest_ptr = *src_ptr;
}