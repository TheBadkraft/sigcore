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
 * File: memory.h
 * Description: Header file for SigmaCore memory management and core interfaces
 *
 */
#pragma once

#include "sigcore/types.h"

/* Public interface for memory operations                        */
/* ============================================================= */
typedef struct sc_memory_i {
   /**
    * @brief Allocate a block of memory of the specified size.
    * @param size Size of memory to allocate in bytes
    * @return Pointer to the allocated memory block
    */
   object (*alloc)(usize);
   /**
    * @brief Free a previously allocated block of memory.
    * @param ptr Pointer to the memory block to free
    */
   void (*free)(object);
   /**
    * @brief Check if a given memory pointer is currently allocated.
    * @param ptr Pointer to check
    * @return true if allocated; otherwise false
    */
   bool (*has)(object);
} sc_memory_i;
extern const sc_memory_i Memory;