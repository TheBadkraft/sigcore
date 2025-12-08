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
 * File: collections.c
 * Description: Source file for SigmaCore collections definitions and interfaces
 *
 * Collections: The core generic collection structures used within SigmaCore.
 *              This includes Iterator and query mechanisms that operate across
 *              all collection types.
 */
#include "sigcore/collections.h"
#include "sigcore/internal/collections.h"
#include "sigcore/types.h"
#include <string.h>

// forward declarations of internal functions
static usize collections_compact(array arr);

// compact an array by removing null entries and compacting non-null to front
static usize collections_compact(array arr) {
   //   return 0 means nothing moved, non-zero is count of moved entries (size)
   if (!arr) {
      return 0; // invalid array
   }

   int cap = Array.capacity(arr);
   usize write_index = 0;

   // iterate through the array, move non-empty entries to front
   for (int i = 0; i < cap; ++i) {
      addr entry;
      if (Array.get(arr, i, &entry) == 0 && entry != ADDR_EMPTY) {
         Array.set(arr, (int)write_index++, entry);
      }
   }

   // set remaining entries to empty
   for (usize i = write_index; i < (usize)cap; ++i) {
      Array.set(arr, (int)i, ADDR_EMPTY);
   }

   // do not shrink the underlying array; maintain capacity

   return write_index; // return count of non-empty elements
}

// public interface implementation
const sc_collections_i Collections = {
    .compact = collections_compact,
};