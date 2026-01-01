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
 * File: memory.c
 * Description: SigmaCore memory management implementation
 */
#include "sigcore/memory.h"
#include "internal/memory_internal.h"
#include "sigcore/arena.h"
#include "sigcore/parray.h"
#include "sigcore/scope.h"
#include "sigcore/slotarray.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations for arena functions
extern arena arena_create(usize initial_pages);
extern void arena_dispose(arena arena);

// Memory allocation hooks - used internally for page management
static void *(*memory_malloc_hook)(size_t) = malloc;
static void (*memory_free_hook)(void *) = free;

struct block {
   usize size;
   struct block *next_free;
   struct block *prev_free;
   // Allocation tracking (used when block is allocated)
   object ptr;               // User pointer
   usize alloc_size;         // Size of user allocation
   struct block *next_alloc; // Next in scope's allocation list
   struct block *prev_alloc; // Prev in scope's allocation list
};

struct sc_pool {
   char handle[4];
   struct sc_page *pages;
   struct block *free_head;
   usize total_bytes;
   usize used_bytes;
};

struct sc_page {
   struct sc_page *next;
   char data[4096];
};

static struct sc_pool root_pool;

// Find and allocate a block from the free list that fits the total_size
static object memory_alloc_from_free(usize total_size, usize size, bool zero) {
   struct block *b = root_pool.free_head;
   while (b) {
      if (b->size >= total_size) {
         if (b->size > total_size) {
            // Split the block
            struct block *split = (struct block *)((char *)b + total_size);
            split->size = b->size - total_size;
            split->next_free = b->next_free;
            split->prev_free = b->prev_free;
            if (b->prev_free)
               b->prev_free->next_free = split;
            else
               root_pool.free_head = split;
            if (b->next_free)
               b->next_free->prev_free = split;
            b->size = total_size;
         } else {
            // Exact fit - remove from free list
            if (b->prev_free)
               b->prev_free->next_free = b->next_free;
            else
               root_pool.free_head = b->next_free;
            if (b->next_free)
               b->next_free->prev_free = b->prev_free;
         }
         b->next_free = NULL;
         b->prev_free = NULL;
         root_pool.used_bytes += total_size - sizeof(struct block);
         object ptr = (char *)b + sizeof(struct block);
         if (zero)
            memset(ptr, 0, size);
         return ptr;
      }
      b = b->next_free;
   }
   return NULL; // No suitable block found
}

// Grow the pool by allocating a new page
static void memory_grow_pool(void) {
   struct sc_page *new_pg = (struct sc_page *)memory_malloc_hook(sizeof(struct sc_page));
   memset(new_pg, 0, sizeof(*new_pg));
   new_pg->next = root_pool.pages;
   root_pool.pages = new_pg;
   root_pool.total_bytes += 4096;

   struct block *new_b = (struct block *)new_pg->data;
   new_b->size = 4096;
   new_b->next_free = NULL;
   new_b->prev_free = NULL;

   // Insert into sorted free list
   struct block *curr = root_pool.free_head;
   struct block *prev = NULL;
   while (curr && (char *)curr < (char *)new_b) {
      prev = curr;
      curr = curr->next_free;
   }
   new_b->next_free = curr;
   new_b->prev_free = prev;
   if (prev)
      prev->next_free = new_b;
   else
      root_pool.free_head = new_b;
   if (curr)
      curr->prev_free = new_b;
}

// allocate a block of memory of the specified size
object memory_alloc(usize size, bool zero) {
   if (size == 0)
      return NULL;

   usize total_size = size + sizeof(struct block);
   total_size = (total_size + 7) & ~7; // Align to 8 bytes

   // Try to allocate from existing free blocks
   object ptr = memory_alloc_from_free(total_size, size, zero);
   if (ptr)
      return ptr;

   // No suitable free block found, grow the pool
   memory_grow_pool();

   // Try again with the new page
   return memory_alloc_from_free(total_size, size, zero);
}

// dispose of a previously allocated block of memory
void memory_dispose(object ptr) {
   if (!ptr)
      return;
   struct block *b = (struct block *)((char *)ptr - sizeof(struct block));
   memset(ptr, 0, b->size - sizeof(struct block));
   root_pool.used_bytes -= b->size - sizeof(struct block);
   b->next_free = NULL;
   b->prev_free = NULL;
   struct block *curr = root_pool.free_head;
   struct block *prev = NULL;
   while (curr && curr < b) {
      prev = curr;
      curr = curr->next_free;
   }
   b->next_free = curr;
   b->prev_free = prev;
   if (prev)
      prev->next_free = b;
   else
      root_pool.free_head = b;
   if (curr)
      curr->prev_free = b;
   if (curr && (char *)b + b->size == (char *)curr) {
      b->size += curr->size;
      b->next_free = curr->next_free;
      if (curr->next_free)
         curr->next_free->prev_free = b;
   }
   if (prev && (char *)prev + prev->size == (char *)b) {
      prev->size += b->size;
      prev->next_free = b->next_free;
      if (b->next_free)
         b->next_free->prev_free = prev;
      b = prev; // Update b to the merged block
   }

   // Check if entire page is free
   if (b->size == 4096 && root_pool.total_bytes > 16 * 4096) {
      // Find and free the page
      struct sc_page *pg = root_pool.pages;
      struct sc_page *prev_pg = NULL;
      while (pg) {
         if ((char *)pg->data <= (char *)b && (char *)b < (char *)pg->data + 4096) {
            // Remove from list
            if (prev_pg)
               prev_pg->next = pg->next;
            else
               root_pool.pages = pg->next;
            // Remove b from free chain
            if (b->prev_free)
               b->prev_free->next_free = b->next_free;
            else
               root_pool.free_head = b->next_free;
            if (b->next_free)
               b->next_free->prev_free = b->prev_free;
            // Free page
            root_pool.total_bytes -= 4096;
            memory_free_hook(pg);
            break;
         }
         prev_pg = pg;
         pg = pg->next;
      }
   }
}

// reallocate memory
static object memory_realloc(object ptr, usize new_size) {
   if (new_size == 0) {
      Memory.dispose(ptr);
      return NULL;
   }
   if (ptr == NULL) {
      return Memory.alloc(new_size, false);
   }
   // Get old size
   struct block *b = (struct block *)((char *)ptr - sizeof(struct block));
   usize old_size = b->size - sizeof(struct block);
   object new_ptr = Memory.alloc(new_size, false);
   if (!new_ptr)
      return NULL;
   usize copy_size = old_size < new_size ? old_size : new_size;
   memcpy(new_ptr, ptr, copy_size);
   Memory.dispose(ptr);
   return new_ptr;
}

// create a new arena
static arena memory_create_arena(usize initial_pages) {
   return arena_create(initial_pages);
}

// dispose an arena
static void memory_dispose_arena(arena arena) {
   arena_dispose(arena);
}

// dispose a memory pool
static void pool_dispose(pool p) {
   if (!p)
      return;

   // Free all pages
   struct sc_page *pg = p->pages;
   while (pg) {
      struct sc_page *next = pg->next;
      memory_free_hook(pg);
      pg = next;
   }

   // Free the pool structure
   memory_free_hook(p);
}

// Grow a pool by allocating a new page
static void pool_grow(pool p) {
   if (!p)
      return;

   struct sc_page *new_pg = (struct sc_page *)memory_malloc_hook(sizeof(struct sc_page));
   if (!new_pg)
      return;

   memset(new_pg, 0, sizeof(*new_pg));
   new_pg->next = p->pages;
   p->pages = new_pg;
   p->total_bytes += 4096;

   struct block *new_b = (struct block *)new_pg->data;
   new_b->size = 4096;
   new_b->next_free = p->free_head;
   new_b->prev_free = NULL;
   if (p->free_head)
      p->free_head->prev_free = new_b;
   p->free_head = new_b;
}

// Allocate from a specific pool
object pool_alloc(pool p, usize size, bool zero) {
   if (!p || size == 0)
      return NULL;

   usize total_size = size + sizeof(struct block);
   if (total_size < size) // Overflow check
      return NULL;

   // Find a suitable block in the free list
   struct block *b = p->free_head;
   while (b) {
      if (b->size >= total_size) {
         if (b->size > total_size) {
            // Split the block
            struct block *split = (struct block *)((char *)b + total_size);
            split->size = b->size - total_size;
            split->next_free = b->next_free;
            split->prev_free = b->prev_free;
            if (b->prev_free)
               b->prev_free->next_free = split;
            else
               p->free_head = split;
            if (b->next_free)
               b->next_free->prev_free = split;
            b->size = total_size;
         } else {
            // Exact fit - remove from free list
            if (b->prev_free)
               b->prev_free->next_free = b->next_free;
            else
               p->free_head = b->next_free;
            if (b->next_free)
               b->next_free->prev_free = b->prev_free;
         }
         b->next_free = NULL;
         b->prev_free = NULL;
         p->used_bytes += total_size - sizeof(struct block);
         object ptr = (char *)b + sizeof(struct block);
         // Initialize allocation tracking
         b->ptr = ptr;
         b->alloc_size = size;
         b->next_alloc = NULL;
         b->prev_alloc = NULL;
         if (zero)
            memset(ptr, 0, size);
         return ptr;
      }
      b = b->next_free;
   }

   // No suitable block found - grow the pool
   pool_grow(p);

   // Try again with the new page
   b = p->free_head;
   while (b) {
      if (b->size >= total_size) {
         if (b->size > total_size) {
            // Split the block
            struct block *split = (struct block *)((char *)b + total_size);
            split->size = b->size - total_size;
            split->next_free = b->next_free;
            split->prev_free = b->prev_free;
            if (b->prev_free)
               b->prev_free->next_free = split;
            else
               p->free_head = split;
            if (b->next_free)
               b->next_free->prev_free = split;
            b->size = total_size;
         } else {
            // Exact fit - remove from free list
            if (b->prev_free)
               b->prev_free->next_free = b->next_free;
            else
               p->free_head = b->next_free;
            if (b->next_free)
               b->next_free->prev_free = b->prev_free;
         }
         b->next_free = NULL;
         b->prev_free = NULL;
         p->used_bytes += total_size - sizeof(struct block);
         object ptr = (char *)b + sizeof(struct block);
         // Initialize allocation tracking
         b->ptr = ptr;
         b->alloc_size = size;
         b->next_alloc = NULL;
         b->prev_alloc = NULL;
         if (zero)
            memset(ptr, 0, size);
         return ptr;
      }
      b = b->next_free;
   }

   // Still no suitable block (shouldn't happen after growing)
   return NULL;
}

// Free memory back to a specific pool
void pool_free(pool p, object ptr) {
   if (!p || !ptr)
      return;

   // Get the block header
   struct block *b = (struct block *)((char *)ptr - sizeof(struct block));

   // Clear allocation tracking
   b->ptr = NULL;
   b->alloc_size = 0;
   b->next_alloc = NULL;
   b->prev_alloc = NULL;

   // Add back to free list
   b->next_free = p->free_head;
   b->prev_free = NULL;
   if (p->free_head)
      p->free_head->prev_free = b;
   p->free_head = b;
   p->used_bytes -= b->size - sizeof(struct block);
}

// create a new memory pool
static pool pool_create(usize initial_pages) {
   if (initial_pages == 0)
      return NULL;

   // Allocate the pool structure
   struct sc_pool *p = (struct sc_pool *)memory_malloc_hook(sizeof(struct sc_pool));
   if (!p)
      return NULL;

   memset(p, 0, sizeof(*p));
   memcpy(p->handle, "POL", 4);

   // Allocate initial pages
   for (usize i = 0; i < initial_pages; i++) {
      struct sc_page *pg = (struct sc_page *)memory_malloc_hook(sizeof(struct sc_page));
      if (!pg) {
         // Cleanup on failure
         pool_dispose(p);
         return NULL;
      }
      memset(pg, 0, sizeof(*pg));
      pg->next = p->pages;
      p->pages = pg;
      p->total_bytes += 4096;

      struct block *b = (struct block *)pg->data;
      b->size = 4096;
      b->next_free = p->free_head;
      b->prev_free = NULL;
      if (p->free_head)
         p->free_head->prev_free = b;
      p->free_head = b;
   }

   return p;
}

// Automatic initialization and teardown using GCC constructor/destructor
__attribute__((constructor)) static void memory_auto_init(void) {
   memset(&root_pool, 0, sizeof(root_pool));
   memcpy(root_pool.handle, "POL\0", 4);
   for (usize i = 0; i < 16; i++) {
      struct sc_page *pg = (struct sc_page *)memory_malloc_hook(sizeof(struct sc_page));
      memset(pg, 0, sizeof(*pg));
      pg->next = root_pool.pages;
      root_pool.pages = pg;
      root_pool.total_bytes += 4096;
      struct block *b = (struct block *)pg->data;
      b->size = 4096;
      b->next_free = root_pool.free_head;
      b->prev_free = NULL;
      if (root_pool.free_head)
         root_pool.free_head->prev_free = b;
      root_pool.free_head = b;
   }
}

// Backdoor functions for testing
struct memory_page *memory_get_current_page(void) {
   // Gutted: return NULL
   return NULL;
}

slotarray memory_get_tracker(void) {
   // Gutted: return NULL
   return NULL;
}

usize memory_get_page_count(void) {
   // Gutted: return 0
   return 0;
}

const sc_memory_i Memory = {
    .alloc = memory_alloc,
    .dispose = memory_dispose,
    .realloc = memory_realloc,
    .Scope = {
        .move = scope_move_scopes,
        .import = scope_import,
    },
    .Pool = {
        .create = pool_create,
        .dispose = pool_dispose,
    },
    .Arena = {
        .create = memory_create_arena,
        .dispose = memory_dispose_arena,
    },
};