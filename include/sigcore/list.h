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
 * File: list.h
 * Description: Header file for SigmaCore list definitions and interfaces
 *
 * List:    An ordered collection structure derived from Array that allows
 *          dynamic resizing, element insertion, removal, and retrieval by index,
 *          or by appending to the end.
 */
#pragma once

#include "sigcore/array.h"

// forward declaration of the list structure
struct sc_list;
typedef struct sc_list *list;

/* Public interface for list operations                        */
/* ============================================================ */
typedef struct sc_list_i {
   /**
    * @brief Create a new list with the specified initial capacity.
    * @param capacity Initial list capacity
    */
   list (*new)(int);
   /**
    * @brief Dispose of the list and free associated resources.
    * @param lst The list to dispose of
    */
   void (*dispose)(list);
} sc_list_i;
extern const sc_list_i List;