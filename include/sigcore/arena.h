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
 * File: arena.h
 * Description: Header file for SigmaCore arena memory management
 */
#pragma once

#include "sigcore/types.h"

// Opaque type declarations
typedef struct sc_arena sc_arena;

typedef struct sc_page sc_page;
typedef struct sc_frame sc_frame;
typedef struct sc_arena *arena;
typedef struct sc_frame *frame;

/* Public interface for arena operations                    */
/* ========================================================= */
typedef struct sc_arena_i {
   /**
    * @brief Allocate memory from the arena with optional zero initialization
    * @param arena The arena to allocate from
    * @param size Size of allocation in bytes
    * @param zee If true, zero-initialize the allocated memory
    * @return Pointer to allocated memory, or NULL if allocation fails
    */
   object (*alloc)(arena, usize, bool);
   /**
    * @brief Check if a pointer is tracked by this arena
    * @param arena The arena to check
    * @param ptr The pointer to verify
    * @return true if the pointer is tracked by the arena
    */
   bool (*is_tracking)(arena, object);
   /**
    * @brief Track a pointer for management by this arena
    * @param arena The arena to track the pointer
    * @param ptr The pointer to track
    */
   void (*track)(arena, object);
   /**
    * @brief Untrack a previously tracked pointer
    * @param arena The arena to untrack the pointer from
    * @param ptr The pointer to untrack
    */
   void (*untrack)(arena, object);
   /**
    * @brief Get the total number of pages in the arena
    * @param arena The arena to query
    * @return Number of pages
    */
   usize (*get_page_count)(arena arena);
   /**
    * @brief Get the total bytes allocated across all pages
    * @param arena The arena to query
    * @return Total allocated bytes
    */
   usize (*get_total_allocated)(arena);
   /**
    * @brief Begin a new frame for temporary allocations
    * @param arena The arena to create a frame for
    * @return A frame handle for temporary allocations, or NULL on failure
    */
   frame (*begin_frame)(arena);
   /**
    * @brief End a frame, freeing all allocations made since begin_frame
    * @param frame The frame handle returned by begin_frame
    */
   void (*end_frame)(frame);
} sc_arena_i;
extern const sc_arena_i Arena;