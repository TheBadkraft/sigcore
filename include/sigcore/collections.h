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
 * File: collections.h
 * Description: Header file for SigmaCore collections definitions and interfaces
 *
 * Collections: The core generic collection structures used within SigmaCore.
 *              This includes Iterator and query mechanisms that operate across
 *              all collection types.
 */
#pragma once

#include "sigcore/collection.h"
#include "sigcore/farray.h"
#include "sigcore/list.h"
#include "sigcore/parray.h"

/* Forward declarations */
typedef struct iterator_s *iterator;

/* Public interface for collections operations                */
/* ============================================================ */
typedef struct sc_collections_i {
   /**
    * @brief Add an element to the collection.
    * @param coll The collection to add to
    * @param ptr Pointer to the element to add
    * @return 0 on OK; otherwise non-zero
    */
   int (*add)(collection, object);
   /**
    * @brief Remove an element from the collection.
    * @param coll The collection to remove from
    * @param ptr Pointer to the element to remove
    * @return 0 on OK; otherwise non-zero
    */
   int (*remove)(collection, object);
   /**
    * @brief Clear all elements from the collection.
    * @param coll The collection to clear
    */
   void (*clear)(collection);
   /**
    * @brief Get the number of elements in the collection.
    * @param coll The collection to query
    * @return Number of elements
    */
   usize (*count)(collection);
   /**
    * @brief Create an iterator for the collection.
    * @param coll The collection to iterate over
    * @return New iterator instance, or NULL on failure
    */
   iterator (*create_iterator)(collection);
   /**
    * @brief Create a collection view of array data.
    * @param buffer Start of the array
    * @param end End of the array (one past last element)
    * @param stride Size of each element
    * @param length Number of elements
    * @param owns_buffer Whether the collection owns the buffer
    * @return New collection instance, or NULL on failure
    */
   collection (*create_view)(void *buffer, void *end, usize stride, usize length, bool owns_buffer);
   /**
    * @brief Dispose of the collection and free its resources.
    * @param coll The collection to dispose
    */
   void (*dispose)(collection);
} sc_collections_i;
extern const sc_collections_i Collections;

/* New Iterator interface - simplified */

typedef struct sc_iterator_i {
   object (*next)(iterator);    /**< Advances and returns the next item, or NULL if none */
   object (*current)(iterator); /**< Returns the current item without advancing, or NULL if none */
   void (*reset)(iterator);     /**< Resets the iterator to the start */
   void (*dispose)(iterator);   /**< Disposes the iterator */
} sc_iterator_i;

extern const sc_iterator_i Iterator;