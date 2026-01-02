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
 * File: memory.h
 * Description: Header file for SigmaCore memory management and core interfaces
 *
 */
#pragma once

#include "sigcore/arena.h"
#include "sigcore/types.h"

// Opaque pool type
typedef struct sc_pool *pool;

/* Public interface for memory operations                        */
/* ============================================================= */
typedef struct sc_memory_i {
   /**
    * @brief Initialize the memory system. Must be called before using any memory operations.
    * @return 0 on success, -1 on failure
    */
   int (*init)(void);
   /**
    * @brief Allocate a block of memory of the specified size.
    * @param size Size of memory to allocate in bytes
    * @param zero If true, zero-initialize the memory; otherwise, leave uninitialized
    * @return Pointer to the allocated memory block
    */
   object (*alloc)(usize, bool);
   /**
    * @brief Dispose of a previously allocated block of memory.
    * @param ptr Pointer to the memory block to dispose
    */
   void (*dispose)(object);
   /**
    * @brief Reallocate a block of memory to a new size.
    * @param ptr Pointer to the memory block to reallocate
    * @param new_size New size in bytes
    * @return Pointer to the reallocated memory block, or NULL on failure
    */
   object (*realloc)(object, usize);

   /**
    * @brief Scope management operations for transferring object ownership between scopes.
    */
   struct {
      /**
       * @brief Get the current active scope for allocations.
       * @return Current scope, or NULL if no scope is active
       */
      void *(*get_current)(void);
      /**
       * @brief Set the current active scope for allocations.
       * @param scope The scope to make current, or NULL to clear
       */
      void (*set_current)(void *scope);
      /**
       * @brief Transfer ownership of an object from one scope to another.
       * @param from Source scope
       * @param to Destination scope
       * @param obj Object to transfer
       * @return 0 on success, -1 on failure
       */
      int (*move)(void *from, void *to, object obj);
      /**
       * @brief Import external data into a scope by copying it.
       * @param scope The scope to import into
       * @param data Pointer to external data to copy
       * @param size Size of data to copy in bytes
       * @return Pointer to the copied data in scope memory, or NULL if import fails
       */
      object (*import)(void *scope, const void *data, usize size);
      /**
       * @brief Export data from a scope to external memory by copying it.
       * @param scope The scope to export from
       * @param data Pointer to scope data to copy
       * @param size Size of data to copy in bytes
       * @return Pointer to the copied data in external memory, or NULL if export fails
       */
      object (*export)(void *scope, const void *data, usize size);
   } Scope;

   /**
    * @brief Pool operations (v2 core memory pools).
    */
   struct {
      /**
       * @brief Create a new memory pool with initial pages.
       * @param initial_pages Number of initial pages
       * @return Pool handle, or NULL on failure
       */
      pool (*create)(usize initial_pages);
      /**
       * @brief Destroy a memory pool.
       * @param p Pool to destroy
       */
      void (*dispose)(pool p);
   } Pool;

   /**
    * @brief Arena operations (still useful for transient frames).
    */
   struct {
      /**
       * @brief Create a new arena with initial pages.
       * @param initial_pages Number of initial pages
       * @return Arena handle, or NULL on failure
       */
      arena (*create)(usize initial_pages);
      /**
       * @brief Dispose an arena.
       * @param a Arena to dispose
       */
      void (*dispose)(arena a);
   } Arena;

} sc_memory_i;
extern const sc_memory_i Memory;