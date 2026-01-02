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
 * File: arena.c
 * Description: SigmaCore arena memory management implementation
 */
#include "sigcore/arena.h"
#include "internal/memory_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Page data size constant (4KB as per design)
#define PAGE_DATA_SIZE 4096

// Internal page structure
struct sc_page {
   struct sc_page *next;      // Chain to next page
   void *bump;                // Current bump pointer
   usize used;                // Bytes used in data area
   char data[PAGE_DATA_SIZE]; // Data area for allocations
};

// Internal arena structure
struct sc_arena {
   char handle[4];           // Scope identifier: "ARN\0"
   sc_page *root_pages;      // Head of permanent pages chain
   sc_page *current_page;    // Active page for allocations
   struct block *alloc_head; // Head of allocation block list
   struct block *alloc_tail; // Tail of allocation block list
   usize page_count;         // Total number of pages
   frame frame_stack;        // Stack of active frames (linked list)
   pool root_pool;           // Root pool for block allocation
};

// Internal frame structure
struct sc_frame {
   char handle[4];           // Scope identifier: "FRM\0"
   sc_arena *arena;          // Arena this frame belongs to
   sc_page *start_page;      // Page where frame began
   void *bump_start;         // Bump pointer position at frame start
   struct block *tail_start; // Allocation tail at frame start
   frame next;               // Next frame in stack (more nested)
   bool valid;               // Whether this frame is still valid (not disposed)
};

#if 1 // Region: Forward declarations
// API functions
static object arena_alloc(arena arena, usize size, bool zero);
static bool arena_is_tracking(arena arena, object ptr);
static void arena_track(arena arena, object ptr);
static void arena_untrack(arena arena, object ptr);
static usize arena_get_page_count(arena arena);
static usize arena_get_total_allocated(arena arena);
static frame arena_begin_frame(arena arena);
static void arena_end_frame(frame current_frame);

// Helper/utility functions
static sc_page *page_create(void);
static void page_destroy(sc_page *page);
static object page_alloc(sc_page *page, usize size, bool zero);
static void arena_add_block(arena arena, struct block *block);
static void arena_remove_block(arena arena, struct block *block);

// Validation helpers
static bool arena_validate(arena arena);
static bool arena_validate_ptr(arena arena, object ptr);
static bool frame_validate(frame current_frame);

// Block management helpers
static struct block *arena_find_block(arena arena, object ptr);
static struct block *arena_create_block(arena arena, object ptr, usize alloc_size);

// Allocation helpers
static bool arena_alloc_validate_and_check_size(arena arena, usize size);
static bool arena_alloc_ensure_page(arena arena);
static object arena_alloc_try_current_page(arena arena, usize size, bool zero);
static object arena_alloc_create_new_page(arena arena, usize size, bool zero);

// Page iterator helper
static usize arena_sum_page_usage(arena arena);

// Frame management helpers
static void arena_end_frame_cleanup_inner_frames(arena arena, frame target_frame);
static void arena_end_frame_reset_bump_pointer(frame current_frame, arena arena);
static void arena_end_frame_cleanup_tracking(frame current_frame, arena arena);
#endif

// Create a new arena
arena arena_create(usize initial_pages) {
   arena arena = memory_alloc(sizeof(sc_arena), true);
   if (!arena)
      return NULL;

   // Initialize handle
   memcpy(arena->handle, "ARN", 4);

   arena->root_pages = NULL;
   arena->current_page = NULL;
   arena->alloc_head = NULL;
   arena->alloc_tail = NULL;
   arena->page_count = 0;
   arena->frame_stack = NULL;
   arena->root_pool = pool_create(1); // Create root pool for block allocation

   // Create initial pages
   for (usize i = 0; i < initial_pages; i++) {
      sc_page *new_page = page_create();
      if (!new_page) {
         // Cleanup on failure
         arena_dispose(arena);
         return NULL;
      }

      new_page->next = arena->root_pages;
      arena->root_pages = new_page;

      if (!arena->current_page) {
         arena->current_page = new_page;
      }

      arena->page_count++;
   }

   return arena;
}

// Destroy an arena
void arena_dispose(arena arena) {
   if (!arena)
      return;

   // Dispose all allocation blocks
   struct block *block = arena->alloc_head;
   while (block) {
      struct block *next = block->next_alloc;
      pool_free(arena->root_pool, (object)block); // Free block back to arena's pool
      block = next;
   }

   // Dispose the root pool
   if (arena->root_pool) {
      pool_dispose(arena->root_pool);
   }

   // Destroy all pages
   sc_page *page = arena->root_pages;
   while (page) {
      sc_page *next = page->next;
      page_destroy(page);
      page = next;
   }

   memory_dispose(arena);
}

// Allocate from arena
static object arena_alloc(arena arena, usize size, bool zero) {
   if (!arena_alloc_validate_and_check_size(arena, size))
      return NULL;

   if (!arena_alloc_ensure_page(arena))
      return NULL;

   // Try to allocate from current page
   object ptr = arena_alloc_try_current_page(arena, size, zero);
   if (ptr)
      return ptr;

   // Current page is full, create new page and allocate
   return arena_alloc_create_new_page(arena, size, zero);
}

// Check if arena is tracking a pointer
static bool arena_is_tracking(arena arena, object ptr) {
   return arena_find_block(arena, ptr) != NULL;
}

// Track a pointer in the arena
static void arena_track(arena arena, object ptr) {
   struct block *block = arena_create_block(arena, ptr, 0); // 0 for external pointers
   if (block) {
      arena_add_block(arena, block);
   }
}

// Untrack a pointer from the arena
static void arena_untrack(arena arena, object ptr) {
   struct block *block = arena_find_block(arena, ptr);
   if (block) {
      arena_remove_block(arena, block);
      pool_free(arena->root_pool, (object)block);
   }
}

// Get page count
static usize arena_get_page_count(arena arena) {
   return arena_validate(arena) ? arena->page_count : 0;
}

// Get total allocated bytes
static usize arena_get_total_allocated(arena arena) {
   return arena_sum_page_usage(arena);
}

// Begin a new frame
static frame arena_begin_frame(arena arena) {
   if (!arena_validate(arena))
      return NULL;

   sc_frame *frame = memory_alloc(sizeof(sc_frame), true);
   if (!frame)
      return NULL;

   // Initialize handle
   memcpy(frame->handle, "FRM", 4);

   frame->arena = arena;
   frame->start_page = arena->current_page;
   frame->bump_start = arena->current_page ? arena->current_page->bump : NULL;
   frame->tail_start = arena->alloc_tail; // Track allocation tail at frame start
   frame->valid = true;

   // Push frame onto stack (most nested frame is at head)
   frame->next = arena->frame_stack;
   arena->frame_stack = frame;

   return frame;
}

// End a frame, rolling back allocations
static void arena_end_frame(frame current_frame) {
   if (!frame_validate(current_frame))
      return;

   sc_arena *arena = current_frame->arena;

   // Check if this frame is at the top of the stack
   if (arena->frame_stack != current_frame) {
      // Frame is not the most recently begun - this is an error
      // For safety, we'll still clean up but this indicates improper nesting
      // In a real implementation, this might be an assertion failure
      fprintf(stderr, "Warning: Ending frame out of order - inner frames will be cleaned up\n");
   }

   // Automatically end any inner frames (more nested) first
   arena_end_frame_cleanup_inner_frames(arena, current_frame);

   // Now handle the target frame
   if (arena->frame_stack == current_frame) {
      arena->frame_stack = current_frame->next; // Pop this frame from stack

      arena_end_frame_reset_bump_pointer(current_frame, arena);

      // Clean up tracking entries if on same page
      if (current_frame->start_page && current_frame->start_page == arena->current_page) {
         arena_end_frame_cleanup_tracking(current_frame, arena);
      }

      current_frame->valid = false;  // Mark as invalid before disposing
      memory_dispose(current_frame); // Free the frame structure
   }
}

// Public interface
const sc_arena_i Arena = {
    .alloc = arena_alloc,
    .is_tracking = arena_is_tracking,
    .track = arena_track,
    .untrack = arena_untrack,
    .get_page_count = arena_get_page_count,
    .get_total_allocated = arena_get_total_allocated,
    .begin_frame = arena_begin_frame,
    .end_frame = arena_end_frame,
};

// Helper/utility function definitions
#if 1 // Region: Helper/utility function definitions
// Validation helpers
static bool arena_validate(arena arena) {
   return arena != NULL;
}

static bool arena_validate_ptr(arena arena, object ptr) {
   return arena_validate(arena) && ptr != NULL;
}

static bool frame_validate(frame current_frame) {
   return current_frame != NULL &&
          current_frame->arena != NULL &&
          current_frame->valid;
}

// Block management helpers
static struct block *arena_find_block(arena arena, object ptr) {
   if (!arena_validate_ptr(arena, ptr)) {
      return NULL;
   }

   struct block *block = arena->alloc_head;
   while (block) {
      if (block->ptr == ptr) {
         return block;
      }
      block = block->next_alloc;
   }
   return NULL;
}

static struct block *arena_create_block(arena arena, object ptr, usize alloc_size) {
   if (!arena_validate(arena)) {
      return NULL;
   }

   struct block *block = (struct block *)pool_alloc(arena->root_pool, sizeof(struct block), false);
   if (block) {
      block->ptr = ptr;
      block->alloc_size = alloc_size;
      block->next_alloc = NULL;
      block->prev_alloc = NULL;
   }
   return block;
}

// Allocation helpers
static bool arena_alloc_validate_and_check_size(arena arena, usize size) {
   if (!arena_validate(arena))
      return false;

   // Handle large allocations (> PAGE_DATA_SIZE) - not supported in bump allocation
   if (size > PAGE_DATA_SIZE)
      return false; // Large allocations are not supported

   return true;
}

static bool arena_alloc_ensure_page(arena arena) {
   // If no current page, create one
   if (!arena->current_page) {
      sc_page *new_page = page_create();
      if (!new_page)
         return false;

      arena->root_pages = new_page;
      arena->current_page = new_page;
      arena->page_count++;
   }
   return true;
}

static object arena_alloc_try_current_page(arena arena, usize size, bool zero) {
   // Try to allocate from current page
   object ptr = page_alloc(arena->current_page, size, zero);
   if (ptr) {
      // Create a block to track this allocation
      struct block *block = arena_create_block(arena, ptr, size);
      if (block) {
         arena_add_block(arena, block);
      }
   }
   return ptr;
}

static object arena_alloc_create_new_page(arena arena, usize size, bool zero) {
   // Current page is full, create new page
   sc_page *new_page = page_create();
   if (!new_page)
      return NULL;

   // Chain the new page
   new_page->next = arena->root_pages;
   arena->root_pages = new_page;
   arena->current_page = new_page;
   arena->page_count++;

   // Allocate from the new page
   object ptr = page_alloc(new_page, size, zero);
   if (ptr) {
      // Create a block to track this allocation
      struct block *block = arena_create_block(arena, ptr, size);
      if (block) {
         arena_add_block(arena, block);
      }
   }

   return ptr;
}

// Page iterator helper
static usize arena_sum_page_usage(arena arena) {
   if (!arena_validate(arena)) {
      return 0;
   }

   usize total = 0;
   sc_page *page = arena->root_pages;
   while (page) {
      total += page->used;
      page = page->next;
   }
   return total;
}

// Frame management helpers
static void arena_end_frame_cleanup_inner_frames(arena arena, frame target_frame) {
   while (arena->frame_stack && arena->frame_stack != target_frame) {
      frame inner_frame = arena->frame_stack;
      arena->frame_stack = inner_frame->next; // Pop inner frame

      // Reset to inner frame's start position
      if (inner_frame->start_page && inner_frame->start_page == arena->current_page) {
         inner_frame->start_page->bump = inner_frame->bump_start;
         if (inner_frame->bump_start >= (void *)inner_frame->start_page->data) {
            inner_frame->start_page->used = (char *)inner_frame->bump_start - inner_frame->start_page->data;
         }
      }

      inner_frame->valid = false;  // Mark as invalid before disposing
      memory_dispose(inner_frame); // Free inner frame
   }
}

static void arena_end_frame_reset_bump_pointer(frame current_frame, arena arena) {
   // Reset bump pointer to frame start
   if (current_frame->start_page && current_frame->start_page == arena->current_page) {
      current_frame->start_page->bump = current_frame->bump_start;
      if (current_frame->bump_start >= (void *)current_frame->start_page->data) {
         current_frame->start_page->used = (char *)current_frame->bump_start - current_frame->start_page->data;
      }
   } else {
      // Frame spanned multiple pages - reset starting page
      if (current_frame->start_page) {
         current_frame->start_page->bump = current_frame->bump_start;
         if (current_frame->bump_start >= (void *)current_frame->start_page->data) {
            current_frame->start_page->used = (char *)current_frame->bump_start - current_frame->start_page->data;
         }
      }
   }
}

static void arena_end_frame_cleanup_tracking(frame current_frame, arena arena) {
   // Clean up tracking entries added during this frame
   // Free allocations from tail_start onward
   struct block *current = current_frame->tail_start;
   while (current) {
      struct block *next = current->next_alloc;
      pool_free(arena->root_pool, current);
      current = next;
   }

   // Update arena's alloc_tail to point before the freed blocks
   arena->alloc_tail = current_frame->tail_start ? current_frame->tail_start->prev_alloc : NULL;
   if (arena->alloc_tail)
      arena->alloc_tail->next_alloc = NULL;
}
#endif

#if 1 // Region: Page helper functions
// Create a new arena page
static sc_page *page_create(void) {
   sc_page *page = malloc(sizeof(sc_page));
   if (!page)
      return NULL;

   page->next = NULL;
   page->bump = page->data;
   page->used = 0;

   return page;
}
// Destroy an arena page
static void page_destroy(sc_page *page) {
   if (!page)
      return;

   free(page);
}
// Allocate from a page
static object page_alloc(sc_page *page, usize size, bool zero) {
   if (!page)
      return NULL;

   // Check if allocation fits
   usize remaining = PAGE_DATA_SIZE - page->used;
   if (size > remaining) {
      return NULL; // Page full or allocation too large
   }

   // Perform bump allocation
   object ptr = (char *)page->bump;
   page->bump = (char *)page->bump + size;
   page->used += size;

   // Zero initialize if requested
   if (zero) {
      memset(ptr, 0, size);
   }

   return ptr;
}
// Helper functions for block list management
static void arena_add_block(arena arena, struct block *block) {
   if (!arena || !block)
      return;

   block->next_alloc = NULL;
   block->prev_alloc = arena->alloc_tail;

   if (arena->alloc_tail) {
      arena->alloc_tail->next_alloc = block;
   } else {
      arena->alloc_head = block;
   }
   arena->alloc_tail = block;
}
static void arena_remove_block(arena arena, struct block *block) {
   if (!arena || !block)
      return;

   if (block->prev_alloc) {
      block->prev_alloc->next_alloc = block->next_alloc;
   } else {
      arena->alloc_head = block->next_alloc;
   }

   if (block->next_alloc) {
      block->next_alloc->prev_alloc = block->prev_alloc;
   } else {
      arena->alloc_tail = block->prev_alloc;
   }

   block->next_alloc = NULL;
   block->prev_alloc = NULL;
}
#endif

#if 1 // Region: Backdoor functions for testing (defined in page_tests.h)
sc_page *Page_create(usize data_size) {
   // For testing, we ignore data_size and use fixed PAGE_DATA_SIZE
   (void)data_size;
   return page_create();
}
void Page_destroy(sc_page *page) {
   page_destroy(page);
}
object Page_alloc(sc_page *page, usize size, bool zero) {
   return page_alloc(page, size, zero);
}
void *Page_get_bump(sc_page *page) {
   return page ? page->bump : NULL;
}
usize Page_get_used(sc_page *page) {
   return page ? page->used : 0;
}
usize Page_get_capacity(sc_page *page) {
   (void)page; // Unused
   return PAGE_DATA_SIZE;
}
slotarray Page_get_tracked_addrs(sc_page *page) {
   (void)page; // No longer tracking at page level
   return NULL;
}
bool Page_contains(sc_page *page, object ptr) {
   if (!page || !ptr)
      return false;
   return ptr >= (object)page->data && ptr < (object)(page->data + PAGE_DATA_SIZE);
}
usize Page_get_allocation_count(sc_page *page) {
   (void)page; // Allocation count no longer tracked per page
   return 0;
}
// Get arena from frame (internal accessor)
arena frame_get_arena(frame f) {
   return f ? f->arena : NULL;
}
#endif