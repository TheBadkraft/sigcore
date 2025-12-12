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
#include "sigcore/internal/memory.h"
#include "sigcore/parray.h"
#include "sigcore/slotarray.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Internal structures
struct sc_slotarray {
   struct {
      void *buffer;
      void *end;
   } array;
   usize stride;
   bool can_grow;
};

struct sc_pointer_array {
   addr *bucket; // pointer to first element (array of addr)
   addr end;     // one past allocated memory (as raw addr)
};

struct index {
   usize head;
   usize next;
};

struct memory_page {
   slotarray slots;
   struct index free_head;
   struct memory_page *next;
   struct memory_page *prev;
};

static struct memory_page *current_page = NULL;
static parray memory_pages = NULL;
static slotarray memory_slots = NULL;
static bool memory_ready = false;

// Static buffers to avoid bootstrap allocations
#define TRACKER_CAPACITY 4096
#define PAGE_ARRAY_CAPACITY 16
#define PAGE_SLOTS_CAPACITY 4096
#define MAX_PAGES 16

// Pre-allocated buffer for the slotarray used as the memory tracker.
// This avoids dynamic allocation during init, preventing self-tracking issues.
static addr page_slots_buffer[PAGE_SLOTS_CAPACITY];
static struct sc_slotarray memory_slots_struct = {
    .array = {.buffer = page_slots_buffer, .end = page_slots_buffer + PAGE_SLOTS_CAPACITY},
    .stride = sizeof(addr),
    .can_grow = false};

// Pre-allocated buffer for the parray holding memory pages.
// Allows static initialization of the page container.
static addr page_bucket[PAGE_ARRAY_CAPACITY];
static struct sc_pointer_array memory_pages_struct = {
    .bucket = page_bucket,
    .end = (addr)(page_bucket + PAGE_ARRAY_CAPACITY)};

// Pre-allocated page structures
static struct memory_page memory_pages_pool[MAX_PAGES];

// The current memory page, encapsulating the tracker slotarray and free head index.
static struct memory_page current_page_struct = {
    .slots = &memory_slots_struct,
    .free_head = {.head = 0, .next = 0},
    .next = NULL,
    .prev = NULL};

static usize next_page_index = 1; // Next available page in pool
static usize page_count = 1;      // Number of active pages

// Create a new memory page with dynamically allocated slotarray buffer
static struct memory_page *create_new_page(void) {
   if (next_page_index >= MAX_PAGES) {
      return NULL; // No more pages available
   }
   struct memory_page *new_page = &memory_pages_pool[next_page_index++];
   page_count++;
   // Allocate buffer for slotarray using raw malloc (not tracked)
   addr *buffer = malloc(PAGE_SLOTS_CAPACITY * sizeof(addr));
   if (!buffer) {
      return NULL;
   }
   // Allocate slotarray struct
   struct sc_slotarray *new_slots = malloc(sizeof(struct sc_slotarray));
   if (!new_slots) {
      free(buffer);
      return NULL;
   }
   new_slots->array.buffer = buffer;
   new_slots->array.end = buffer + PAGE_SLOTS_CAPACITY;
   new_slots->stride = sizeof(addr);
   new_slots->can_grow = true;
   // Initialize all to ADDR_EMPTY
   for (usize i = 0; i < PAGE_SLOTS_CAPACITY; i++) {
      buffer[i] = ADDR_EMPTY;
   }
   new_page->slots = new_slots;
   new_page->free_head = (struct index){0, 0};
   new_page->next = NULL;
   new_page->prev = current_page;
   return new_page;
}

// allocate a block of memory of the specified size
static object memory_alloc(usize size, bool zee) {
   object ptr = malloc(size);
   if (zee && ptr) {
      memset(ptr, 0, size);
   }
   if (memory_ready && current_page) {
      // Try to track the allocation in the current page
      int result = SlotArray.add(current_page->slots, ptr);
      if (result == ERR) {
         // Current page is full, create a new page
         struct memory_page *new_page = create_new_page();
         if (new_page) {
            // Link the new page
            current_page->next = new_page;
            // Add to pages array
            // PArray.set(memory_pages, page_count++, (addr)new_page);  // Temporarily disabled
            // Switch to new page
            current_page = new_page;
            // Now try to add to the new page
            result = SlotArray.add(current_page->slots, ptr);
            if (result == ERR) {
               // New page also failed, free and return NULL
               free(ptr);
               return NULL;
            }
         } else {
            // No more pages, free and return NULL
            free(ptr);
            return NULL;
         }
      }
   }
   return ptr;
}

// dispose of a previously allocated block of memory
static void memory_dispose(object ptr) {
   if (memory_ready) {
      // Traverse all pages to find and remove from tracker
      struct memory_page *page = &current_page_struct; // Start from first page
      while (page) {
         usize cap = SlotArray.capacity(page->slots);
         for (usize i = 0; i < cap; i++) {
            if (!SlotArray.is_empty_slot(page->slots, i)) {
               object val;
               if (SlotArray.get_at(page->slots, i, &val) == 0 && val == ptr) {
                  SlotArray.remove_at(page->slots, i);
                  goto found;
               }
            }
         }
         page = page->next;
      }
   found:;
   }
   free(ptr);
}

// check if a given memory pointer is currently being tracked
static bool memory_is_tracking(object ptr) {
   if (!memory_ready)
      return false;
   // Traverse all pages
   struct memory_page *page = &current_page_struct;
   while (page) {
      usize cap = SlotArray.capacity(page->slots);
      for (usize i = 0; i < cap; i++) {
         if (!SlotArray.is_empty_slot(page->slots, i)) {
            object val;
            if (SlotArray.get_at(page->slots, i, &val) == 0 && val == ptr)
               return true;
         }
      }
      page = page->next;
   }
   return false;
}

// track a memory pointer
static void memory_track(object ptr) {
   if (memory_ready && current_page) {
      int result = SlotArray.add(current_page->slots, ptr);
      if (result == ERR) {
         // Current page full, create new page
         struct memory_page *new_page = create_new_page();
         if (new_page) {
            current_page->next = new_page;
            PArray.set(memory_pages, page_count++, (addr)new_page);
            current_page = new_page;
            SlotArray.add(current_page->slots, ptr);
         }
         // If failed, silently ignore
      }
   }
}

// untrack a memory pointer
static void memory_untrack(object ptr) {
   if (memory_ready) {
      // Traverse all pages
      struct memory_page *page = &current_page_struct;
      while (page) {
         usize cap = SlotArray.capacity(page->slots);
         for (usize i = 0; i < cap; i++) {
            if (!SlotArray.is_empty_slot(page->slots, i)) {
               object val;
               if (SlotArray.get_at(page->slots, i, &val) == 0 && val == ptr) {
                  SlotArray.remove_at(page->slots, i);
                  return;
               }
            }
         }
         page = page->next;
      }
   }
}

// internal function to swap tracked pointers (for realloc optimization)
static bool memory_swap_tracked_pointers(object old_ptr, object new_ptr) {
   if (!memory_ready || !old_ptr || !new_ptr)
      return false;

   // Traverse all pages to find and swap the pointer
   struct memory_page *page = &current_page_struct;
   while (page) {
      usize cap = SlotArray.capacity(page->slots);
      for (usize i = 0; i < cap; i++) {
         if (!SlotArray.is_empty_slot(page->slots, i)) {
            object val;
            if (SlotArray.get_at(page->slots, i, &val) == 0 && val == old_ptr) {
               // Found the old pointer, replace it with new pointer
               addr *slot = (addr *)((char *)page->slots->array.buffer + i * page->slots->stride);
               *slot = (addr)new_ptr;
               return true;
            }
         }
      }
      page = page->next;
   }
   return false; // old_ptr not found
}

// reallocate memory
static object memory_realloc(object ptr, usize new_size) {
   if (!ptr)
      return memory_alloc(new_size, false);
   if (!new_size) {
      memory_dispose(ptr);
      return NULL;
   }

   // Temporary: use dispose+alloc until swap is debugged
   if (memory_is_tracking(ptr)) {
      memory_dispose(ptr);
      return memory_alloc(new_size, false);
   }
   return NULL;
}

// check if memory system is ready
static bool memory_is_ready(void) {
   return memory_ready;
}

// initialize memory system
static void memory_init(void) {
   if (memory_ready)
      return; // already initialized

   // Initialize static structures
   memory_slots_struct.array.buffer = page_slots_buffer;
   memory_slots_struct.array.end = page_slots_buffer + PAGE_SLOTS_CAPACITY;
   memory_slots_struct.stride = sizeof(addr);
   memory_slots_struct.can_grow = false;

   // Initialize all slots to ADDR_EMPTY
   for (usize i = 0; i < PAGE_SLOTS_CAPACITY; i++) {
      page_slots_buffer[i] = ADDR_EMPTY;
   }

   // Set pointers
   memory_pages = &memory_pages_struct;
   current_page = &current_page_struct;
   memory_slots = &memory_slots_struct;

   // Set memory_pages[0] = current_page
   PArray.set(memory_pages, 0, (addr)current_page);

   memory_ready = true;

   // Note: Internal structures are not tracked to avoid self-tracking issues
}

// teardown memory system
static void memory_teardown(void) {
   if (!memory_ready)
      return;

   // Dispose all tracked allocations from all pages
   struct memory_page *page = &current_page_struct;
   while (page) {
      if (page->slots) {
         usize cap = SlotArray.capacity(page->slots);
         for (usize i = 0; i < cap; i++) {
            if (!SlotArray.is_empty_slot(page->slots, i)) {
               object val;
               if (SlotArray.get_at(page->slots, i, &val) == 0) {
                  free(val); // raw free, bypass dispose to avoid recursion
               }
            }
         }
         // Free the buffer and struct if not the static ones
         if (page->slots != &memory_slots_struct) {
            free(page->slots->array.buffer);
            free(page->slots);
         }
      }
      page = page->next;
   }

   // Reset globals
   memory_slots = NULL;
   current_page = NULL;
   memory_pages = NULL;
   next_page_index = 1;
   page_count = 1;
   memory_ready = false;
}

const sc_memory_i Memory = {
    .alloc = memory_alloc,
    .dispose = memory_dispose,
    .realloc = memory_realloc,
    .is_tracking = memory_is_tracking,
    .track = memory_track,
    .untrack = memory_untrack,
    .init = memory_init,
    .teardown = memory_teardown,
    .is_ready = memory_is_ready,
};

// Backdoor functions for testing
struct memory_page *Memory_get_current_page(void) {
   return current_page;
}

slotarray Memory_get_tracker(void) {
   return memory_slots;
}

usize Memory_get_page_count(void) {
   return page_count;
}