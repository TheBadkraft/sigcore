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
 * File:  internal/collections.h
 * Description: Header file for SigmaCore internal collection functions
 */
#pragma once

#include "sigcore/collections.h"
#include "sigcore/parray.h"

/* Collection structure                                        */
/* ============================================================ */
struct sc_collection {
   struct {
      void *buffer;
      void *end;
   } array;
   usize stride;
   usize length;
};

// array internal functions
addr array_get_bucket_start(parray arr);
addr array_get_bucket_end(parray arr);
addr *array_get_bucket(parray arr);

// collection internal functions
collection collection_new(usize capacity, usize stride);
void collection_dispose(collection coll);
int collection_add(collection coll, object ptr);
int collection_grow(collection coll);
void collection_clear(collection coll);