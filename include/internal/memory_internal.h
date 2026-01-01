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
 * File: internal/memory.h
 * Description: Internal memory interfaces for testing and development
 */

#pragma once

#include "sigcore/arena.h"
#include "sigcore/memory.h"
#include "sigcore/types.h"

// Allocation tracking structure
struct sc_allocation {
   addr ptr;
   usize size;
};

// Opaque slotarray typedef for backdoor access
typedef struct sc_slotarray *slotarray;

// Memory page structure (internal)
struct memory_page;

// Memory functions (internal, used by Arena)
object memory_alloc(usize, bool);
void memory_dispose(object);

// Pool functions (internal, used by Arena)
object pool_alloc(pool, usize, bool);
void pool_free(pool, object);

// Arena functions (internal, used by Memory)
arena arena_create(usize);
void arena_dispose(arena);

// Scope functions (internal, used by Memory)
object scope_import(void *, const void *, usize);

// Backdoor functions for testing internals
struct memory_page *memory_get_current_page(void);
slotarray memory_get_tracker(void);
usize memory_get_page_count(void);