/*
 *  Test File: test_mempaging.c
 *  Description: Test cases for SigmaCore memory paging
 */

#include "internal/memory_internal.h"
#include "sigcore/memory.h"
#include <sigtest/sigtest.h>
#include <stdio.h>
#include <stdlib.h>

#define PAGE_SLOTS_CAPACITY 512

static void set_config(FILE **log_stream) {
   // *log_stream = fopen("logs/test_mempaging.log", "w");
   *log_stream = NULL;
   // Set memory hooks to use sigtest's wrapped functions for tracking
   Memory.set_alloc_hooks(__wrap_malloc, __wrap_free, NULL, NULL);
}
static void set_teardown(void) {
   Memory.reset_alloc_hooks();
   Memory.teardown();
}

// test memory multi-page creation
void test_memory_multi_page_creation(void) {
   usize initial_pages = Memory_get_page_count();
   Assert.isTrue(initial_pages == 1, "Memory system should start with exactly 1 page");
}

// test cross-page tracking
void test_memory_cross_page_tracking(void) {
   usize initial_pages = Memory_get_page_count();
   usize num_to_fill = PAGE_SLOTS_CAPACITY - 10;

   // Allocate enough to fill the first page
   void **ptrs = Memory.alloc((num_to_fill + 10) * sizeof(void *), false);
   Assert.isNotNull(ptrs, "Should successfully allocate array to hold test pointers");

   for (usize i = 0; i < num_to_fill + 10; i++) {
      ptrs[i] = Memory.alloc(16, false);
      Assert.isNotNull(ptrs[i], "Allocation %zu (size=16) should succeed", i);
      Assert.isTrue(Memory.is_tracking(ptrs[i]), "Allocation %zu should be tracked by memory system", i);
   }

   usize pages_after = Memory_get_page_count();
   Assert.isTrue(pages_after > initial_pages, "Should have created additional pages after %zu allocations (had %zu pages, now %zu)", num_to_fill + 10, initial_pages, pages_after);

   // Dispose all
   for (usize i = 0; i < num_to_fill + 10; i++) {
      Memory.dispose(ptrs[i]);
   }
   Memory.dispose(ptrs);
}

// test hole filling across pages
void test_memory_hole_filling(void) {
   // Allocate to fill first page
   void **ptrs = Memory.alloc(PAGE_SLOTS_CAPACITY * sizeof(void *), false);
   for (usize i = 0; i < PAGE_SLOTS_CAPACITY; i++) {
      ptrs[i] = Memory.alloc(16, false);
   }

   // Dispose some from first page to create holes
   for (usize i = 0; i < 100; i += 2) { // Dispose every other
      Memory.dispose(ptrs[i]);
      ptrs[i] = NULL;
   }

   // Allocate new ones - should fill holes in existing pages before new page
   usize pages_before = Memory_get_page_count();
   for (usize i = 0; i < 50; i++) {
      void *new_ptr = Memory.alloc(16, false);
      Assert.isNotNull(new_ptr, "New allocation %zu during hole filling should succeed", i);
      Assert.isTrue(Memory.is_tracking(new_ptr), "New allocation %zu should be tracked", i);
      // Find a spot in ptrs to store it
      for (usize j = 0; j < PAGE_SLOTS_CAPACITY; j++) {
         if (ptrs[j] == NULL) {
            ptrs[j] = new_ptr;
            break;
         }
      }
   }

   usize pages_after = Memory_get_page_count();
   Assert.isTrue(pages_before == pages_after, "Page count should remain unchanged during hole filling (%zu pages)", pages_before);

   // Dispose all remaining
   for (usize i = 0; i < PAGE_SLOTS_CAPACITY; i++) {
      if (ptrs[i]) {
         Memory.dispose(ptrs[i]);
      }
   }
   Memory.dispose(ptrs);
}

// test multi-page stress
void test_memory_multi_page_stress(void) {
   usize num_allocs = 1000;
   void **ptrs = Memory.alloc(num_allocs * sizeof(void *), false);
   Assert.isNotNull(ptrs, "Should successfully allocate array for %zu stress test pointers", num_allocs);

   // Allocate many pointers
   for (usize i = 0; i < num_allocs; i++) {
      ptrs[i] = Memory.alloc(32, false);
      Assert.isNotNull(ptrs[i], "Stress allocation %zu (size=32) should succeed", i);
      Assert.isTrue(Memory.is_tracking(ptrs[i]), "Stress allocation %zu should be tracked", i);
   }

   usize pages_used = Memory_get_page_count();
   Assert.isTrue(pages_used > 1, "Stress test with %zu allocations should use multiple pages (using %zu)", num_allocs, pages_used);

   // Dispose some and re-allocate
   for (usize i = 0; i < 100; i++) {
      Memory.dispose(ptrs[i]);
      ptrs[i] = NULL;
   }

   for (usize i = 0; i < 100; i++) {
      void *new_ptr = Memory.alloc(32, false);
      Assert.isNotNull(new_ptr, "Re-allocation %zu after dispose should succeed", i);
      Assert.isTrue(Memory.is_tracking(new_ptr), "Re-allocation %zu should be tracked", i);
      // Store in a free spot
      for (usize j = 0; j < num_allocs; j++) {
         if (ptrs[j] == NULL) {
            ptrs[j] = new_ptr;
            break;
         }
      }
   }

   // Verify all are tracked
   for (usize i = 0; i < num_allocs; i++) {
      Assert.isTrue(Memory.is_tracking(ptrs[i]), "Final pointer %zu should be tracked after stress operations", i);
   }

   // Dispose all
   for (usize i = 0; i < num_allocs; i++) {
      Memory.dispose(ptrs[i]);
   }
   Memory.dispose(ptrs);
}

//  register test cases
__attribute__((constructor)) void init_mempaging_tests(void) {
   testset("core_mempaging_set", set_config, set_teardown);

   testcase("Multi-page creation", test_memory_multi_page_creation);
   testcase("Cross-page tracking", test_memory_cross_page_tracking);
   testcase("Hole filling across pages", test_memory_hole_filling);
   testcase("Multi-page stress test", test_memory_multi_page_stress);
}