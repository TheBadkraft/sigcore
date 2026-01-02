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
 * File:  internal/collections.h
 * Description: Header file for SigmaCore internal collection functions
 */
#pragma once

#include "internal/array_base.h"
#include "sigcore/collections.h"
#include "sigcore/farray.h"
#include "sigcore/parray.h"

// collection structure (internal)
struct sc_collection {
   sc_array_base array;
   usize stride;
   usize length;
   bool owns_buffer;
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
void collection_set_data(collection coll, void *data, usize count);

// collection accessor functions
void *collection_get_buffer(collection coll);
void *collection_get_end(collection coll);
usize collection_get_stride(collection coll);
usize collection_get_length(collection coll);
void collection_set_length(collection coll, usize length);

// array collection helpers
collection array_create_collection_view(void *buffer, void *end, usize stride, usize length, bool owns_buffer);