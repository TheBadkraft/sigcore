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
#include "sigcore/slotarray.h"
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
   slotarray tracked_addrs;   // SlotArray of allocated addresses
   char data[PAGE_DATA_SIZE]; // Data area for allocations
};

// Internal arena structure
struct sc_arena {
   char handle[4];         // Scope identifier: "ARN\0"
   sc_page *root_pages;    // Head of permanent pages chain
   sc_page *current_page;  // Active page for allocations
   slotarray root_tracker; // Global root tracker (for future inheritance)
   usize page_count;       // Total number of pages
   frame frame_stack;      // Stack of active frames (linked list)
};

// Internal frame structure
struct sc_frame {
   char handle[4];      // Scope identifier: "FRM\0"
   sc_arena *arena;     // Arena this frame belongs to
   sc_page *start_page; // Page where frame began
   void *bump_start;    // Bump pointer position at frame start
   usize slots_start;   // Number of slots tracked at frame start
   frame next;          // Next frame in stack (more nested)
   bool valid;          // Whether this frame is still valid (not disposed)
};

// Forward declarations for internal functions
static sc_page *page_create(void);
static void page_destroy(sc_page *page);
static object page_alloc(sc_page *page, usize size, bool zero);

// Create a new arena page
static sc_page *page_create(void) {
   sc_page *page = malloc(sizeof(sc_page));
   if (!page)
      return NULL;

   page->next = NULL;
   page->bump = page->data;
   page->used = 0;
   page->tracked_addrs = SlotArray.new(64);

   if (!page->tracked_addrs) {
      free(page);
      return NULL;
   }

   return page;
}

// Destroy an arena page
static void page_destroy(sc_page *page) {
   if (!page)
      return;

   // Note: tracked_addrs contains pointers to bump-allocated memory within page->data
   // These are freed when the page struct itself is freed, so we don't free them individually
   if (page->tracked_addrs) {
      SlotArray.dispose(page->tracked_addrs);
   }

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

   // Track the allocation
   SlotArray.add(page->tracked_addrs, ptr);

   // Zero initialize if requested
   if (zero) {
      memset(ptr, 0, size);
   }

   return ptr;
}

// Create a new arena
arena arena_create(usize initial_pages) {
   arena arena = memory_alloc(sizeof(sc_arena), true);
   if (!arena)
      return NULL;

   // Initialize handle
   memcpy(arena->handle, "ARN", 4);

   arena->root_pages = NULL;
   arena->current_page = NULL;
   // arena->root_tracker = SlotArray.new(64); // TEMP: disable for testing
   arena->root_tracker = NULL;
   arena->page_count = 0;
   arena->frame_stack = NULL;

   // if (!arena->root_tracker) {
   //    memory_dispose(arena);
   //    return NULL;
   // }

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

   // Destroy all pages
   sc_page *page = arena->root_pages;
   while (page) {
      sc_page *next = page->next;
      page_destroy(page);
      page = next;
   }

   // Dispose root tracker
   // if (arena->root_tracker) {
   //    SlotArray.dispose(arena->root_tracker);
   // }

   memory_dispose(arena);
}

// Allocate from arena
static object arena_alloc(arena arena, usize size, bool zero) {
   if (!arena)
      return NULL;

   // Handle large allocations (> PAGE_DATA_SIZE) - not supported in bump allocation
   if (size > PAGE_DATA_SIZE)
      return NULL; // Large allocations are not supported

   // If no current page, create one
   if (!arena->current_page) {
      sc_page *new_page = page_create();
      if (!new_page)
         return NULL;

      arena->root_pages = new_page;
      arena->current_page = new_page;
      arena->page_count++;
   }

   // Try to allocate from current page
   object ptr = page_alloc(arena->current_page, size, zero);
   if (ptr)
      return ptr;

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
   ptr = page_alloc(new_page, size, zero);
   // Should not fail since we just created a fresh page

   return ptr;
}

// Check if arena is tracking a pointer
static bool arena_is_tracking(arena arena, object ptr) {
   if (!arena || !ptr)
      return false;

   // Search all pages
   sc_page *page = arena->root_pages;
   while (page) {
      // Check if pointer is in this page's data area
      if (ptr >= (object)page->data && ptr < (object)(page->data + PAGE_DATA_SIZE)) {
         // Check if it's tracked in this page's slotarray
         usize cap = SlotArray.capacity(page->tracked_addrs);
         for (usize i = 0; i < cap; i++) {
            if (!SlotArray.is_empty_slot(page->tracked_addrs, i)) {
               object tracked_ptr;
               if (SlotArray.get_at(page->tracked_addrs, i, &tracked_ptr) == 0 &&
                   tracked_ptr == ptr) {
                  return true;
               }
            }
         }
      }
      page = page->next;
   }

   return false;
}

// Track a pointer in the arena
static void arena_track(arena arena, object ptr) {
   if (!arena || !ptr || !arena->current_page)
      return;

   SlotArray.add(arena->current_page->tracked_addrs, ptr);
}

// Untrack a pointer from the arena
static void arena_untrack(arena arena, object ptr) {
   if (!arena || !ptr)
      return;

   // Search all pages for the pointer and remove it
   sc_page *page = arena->root_pages;
   while (page) {
      // Check if pointer is in this page's data area
      if (ptr >= (object)page->data && ptr < (object)(page->data + PAGE_DATA_SIZE)) {
         // Check if it's tracked in this page's slotarray
         usize cap = SlotArray.capacity(page->tracked_addrs);
         for (usize i = 0; i < cap; i++) {
            if (!SlotArray.is_empty_slot(page->tracked_addrs, i)) {
               object tracked_ptr;
               if (SlotArray.get_at(page->tracked_addrs, i, &tracked_ptr) == 0 &&
                   tracked_ptr == ptr) {
                  SlotArray.remove_at(page->tracked_addrs, i);
                  return; // Found and removed
               }
            }
         }
      }
      page = page->next;
   }
}

// Get page count
static usize arena_get_page_count(arena arena) {
   return arena ? arena->page_count : 0;
}

// Get total allocated bytes
static usize arena_get_total_allocated(arena arena) {
   if (!arena)
      return 0;

   usize total = 0;
   sc_page *page = arena->root_pages;
   while (page) {
      total += page->used;
      page = page->next;
   }

   return total;
}

// Begin a new frame
static frame arena_begin_frame(arena arena) {
   if (!arena)
      return NULL;

   sc_frame *frame = memory_alloc(sizeof(sc_frame), true);
   if (!frame)
      return NULL;

   // Initialize handle
   memcpy(frame->handle, "FRM", 4);

   frame->arena = arena;
   frame->start_page = arena->current_page;
   frame->bump_start = arena->current_page ? arena->current_page->bump : NULL;
   frame->slots_start = arena->current_page ? SlotArray.capacity(arena->current_page->tracked_addrs) : 0;
   frame->valid = true;

   // Push frame onto stack (most nested frame is at head)
   frame->next = arena->frame_stack;
   arena->frame_stack = frame;

   return frame;
}

// End a frame, rolling back allocations
static void arena_end_frame(frame current_frame) {
   if (!current_frame || !current_frame->arena || !current_frame->valid)
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
   while (arena->frame_stack && arena->frame_stack != current_frame) {
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

   // Now handle the target frame
   if (arena->frame_stack == current_frame) {
      arena->frame_stack = current_frame->next; // Pop this frame from stack

      // Reset bump pointer to frame start
      if (current_frame->start_page && current_frame->start_page == arena->current_page) {
         current_frame->start_page->bump = current_frame->bump_start;
         if (current_frame->bump_start >= (void *)current_frame->start_page->data) {
            current_frame->start_page->used = (char *)current_frame->bump_start - current_frame->start_page->data;
         }

         // Clean up tracking entries added during this frame
         slotarray tracker = current_frame->start_page->tracked_addrs;
         if (tracker) {
            usize current_slots = SlotArray.capacity(tracker);
            // Remove slots from slots_start onwards
            for (usize i = current_frame->slots_start; i < current_slots;) {
               if (!SlotArray.is_empty_slot(tracker, i)) {
                  SlotArray.remove_at(tracker, i);
                  // Don't increment i since we removed an element
               } else {
                  i++; // Move to next slot
               }
            }
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

// Backdoor functions for testing (defined in page_tests.h)
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
   return page ? page->tracked_addrs : NULL;
}

bool Page_contains(sc_page *page, object ptr) {
   if (!page || !ptr)
      return false;
   return ptr >= (object)page->data && ptr < (object)(page->data + PAGE_DATA_SIZE);
}

usize Page_get_allocation_count(sc_page *page) {
   if (!page || !page->tracked_addrs)
      return 0;

   usize count = 0;
   usize cap = SlotArray.capacity(page->tracked_addrs);
   for (usize i = 0; i < cap; i++) {
      if (!SlotArray.is_empty_slot(page->tracked_addrs, i)) {
         count++;
      }
   }

   return count;
}

// Get arena from frame (internal accessor)
arena frame_get_arena(frame f) {
   return f ? f->arena : NULL;
}