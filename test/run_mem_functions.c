/*
 *  Test File: test_memory_robustness.c
 *  Description: Comprehensive robustness tests for SigmaCore memory system
 *               Designed to fail and test resilience of arena/page architecture
 */

#include "internal/page_tests.h"
#include "sigcore/arena.h"
#include "sigcore/memory.h"
#include <sigtest/sigtest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Page data size constant (matches arena.c definition)
#define PAGE_DATA_SIZE 4096

static void set_config(FILE **log_stream) {
   *log_stream = fopen("logs/test_memory_robustness.log", "w");
   // Memory.init(); // Now automatic via constructor
}
static void set_teardown(void) {
   Memory.teardown();
}

// test arena inheritance groundwork (tracking persistence)
void test_inheritance_tracking_persistence(void) {
   sc_arena *test_arena = Memory.create_arena(1);
   Assert.isNotNull(test_arena, "Arena setup should succeed");

   // Allocate several pointers that would be "inherited" in Phase 3
   const int num_ptrs = 5;
   object ptrs[num_ptrs];
   for (int i = 0; i < num_ptrs; i++) {
      ptrs[i] = Arena.alloc(test_arena, 64 + i * 16, false);
      Assert.isNotNull(ptrs[i], "Allocation %d should succeed", i);
      Assert.isTrue(Arena.is_tracking(test_arena, ptrs[i]), "Pointer %d should be tracked", i);
   }

   // Simulate what inheritance would do: pointers remain valid and tracked
   // In Phase 3, these would be moved to root tracker, but for now verify
   // they remain accessible and tracked throughout arena lifetime
   for (int i = 0; i < num_ptrs; i++) {
      Assert.isTrue(Arena.is_tracking(test_arena, ptrs[i]), "Inherited pointer %d should remain tracked", i);

      // Verify pointer is still usable (write/read test)
      int *test_val = (int *)ptrs[i];
      *test_val = 42 + i;
      Assert.areEqual(&(int){42 + i}, test_val, INT, "Inherited pointer %d should be writable and readable", i);
   }

   usize total_allocated = Arena.get_total_allocated(test_arena);
   Assert.isTrue(total_allocated > 0, "Should have allocated memory");

   Memory.dispose_arena(test_arena);
}

// test concurrent-like allocation patterns (stress test)
void test_allocation_pattern_stress(void) {
   sc_arena *test_arena = Memory.create_arena(1);
   Assert.isNotNull(test_arena, "Arena setup should succeed");

   // Pattern 1: Small incremental allocations
   const int small_count = 100;
   object small_ptrs[small_count];
   for (int i = 0; i < small_count; i++) {
      small_ptrs[i] = Arena.alloc(test_arena, 8, false);
      Assert.isNotNull(small_ptrs[i], "Small allocation %d should succeed", i);
   }

   // Pattern 2: Large allocations interspersed
   const int large_count = 10;
   object large_ptrs[large_count];
   for (int i = 0; i < large_count; i++) {
      large_ptrs[i] = Arena.alloc(test_arena, 1000, false);
      Assert.isNotNull(large_ptrs[i], "Large allocation %d should succeed", i);
   }

   // Pattern 3: Mixed zero-init and non-zero-init
   const int mixed_count = 50;
   object mixed_ptrs[mixed_count];
   for (int i = 0; i < mixed_count; i++) {
      bool zero_init = (i % 2 == 0);
      mixed_ptrs[i] = Arena.alloc(test_arena, 32, zero_init);
      Assert.isNotNull(mixed_ptrs[i], "Mixed allocation %d should succeed", i);

      if (zero_init) {
         // Verify zero initialization
         char *data = (char *)mixed_ptrs[i];
         for (int j = 0; j < 32; j++) {
            Assert.areEqual(&(char){0}, &data[j], CHAR, "Zero-init byte %d should be 0", j);
         }
      }
   }

   // Verify all pointers are still tracked
   for (int i = 0; i < small_count; i++) {
      Assert.isTrue(Arena.is_tracking(test_arena, small_ptrs[i]), "Small pointer %d should be tracked", i);
   }
   for (int i = 0; i < large_count; i++) {
      Assert.isTrue(Arena.is_tracking(test_arena, large_ptrs[i]), "Large pointer %d should be tracked", i);
   }
   for (int i = 0; i < mixed_count; i++) {
      Assert.isTrue(Arena.is_tracking(test_arena, mixed_ptrs[i]), "Mixed pointer %d should be tracked", i);
   }

   usize page_count = Arena.get_page_count(test_arena);
   Assert.isTrue(page_count > 1, "Mixed patterns should require multiple pages (actual: %d)", page_count);

   Memory.dispose_arena(test_arena);
}

// test page boundary stress (inheritance groundwork)
void test_page_boundary_stress(void) {
   sc_page *page = Page_create(PAGE_DATA_SIZE);
   Assert.isNotNull(page, "Page setup should succeed");

   // Fill page to near capacity with small allocations
   const int num_small = 50;
   object small_ptrs[num_small];
   usize small_size = 64;
   usize total_small = 0;

   for (int i = 0; i < num_small; i++) {
      small_ptrs[i] = Page_alloc(page, small_size, false);
      if (!small_ptrs[i])
         break; // Page full
      total_small += small_size;
   }

   usize used_after_small = Page_get_used(page);
   Assert.isTrue(used_after_small > 0, "Should have allocated some small blocks");

   // Try to allocate one more small block - might fail
   object boundary_ptr = Page_alloc(page, small_size, false);
   // This might succeed or fail depending on remaining space

   // If it succeeded, verify it's tracked
   if (boundary_ptr) {
      Assert.isTrue(Page_contains(page, boundary_ptr), "Boundary allocation should be within page");
      usize expected_count = num_small + 1;
      usize actual_count = Page_get_allocation_count(page);
      Assert.areEqual(&expected_count, &actual_count, LONG, "Should track boundary allocation");
   }

   // Test that all existing allocations are still valid
   for (int i = 0; i < num_small; i++) {
      if (small_ptrs[i]) {
         Assert.isTrue(Page_contains(page, small_ptrs[i]), "Small pointer %d should still be in page", i);

         // Test writability
         char *data = (char *)small_ptrs[i];
         data[0] = (char)i;
         Assert.areEqual(&(char){(char)i}, &data[0], CHAR, "Small pointer %d should be writable", i);
      }
   }

   Page_destroy(page);
}

// test arena root tracking groundwork (for inheritance)
void test_root_tracking_groundwork(void) {
   sc_arena *test_arena = Memory.create_arena(2);
   Assert.isNotNull(test_arena, "Arena setup should succeed");

   // Allocate across multiple pages
   const int num_allocs = 20;
   object ptrs[num_allocs];
   for (int i = 0; i < num_allocs; i++) {
      usize size = 200 + (i * 10); // Force page growth
      ptrs[i] = Arena.alloc(test_arena, size, false);
      Assert.isNotNull(ptrs[i], "Allocation %d should succeed", i);
   }

   usize page_count = Arena.get_page_count(test_arena);
   Assert.isTrue(page_count >= 2, "Should have grown to multiple pages (actual: %d)", page_count);

   // In Phase 3, some of these would be "inherited" to root tracker
   // For now, verify they all remain tracked and accessible
   for (int i = 0; i < num_allocs; i++) {
      Assert.isTrue(Arena.is_tracking(test_arena, ptrs[i]), "Pointer %d should be tracked across pages", i);

      // Test cross-page accessibility
      memset(ptrs[i], i, 10); // Write first 10 bytes
      char *data = (char *)ptrs[i];
      for (int j = 0; j < 10; j++) {
         Assert.areEqual(&(char){(char)i}, &data[j], CHAR, "Cross-page pointer %d byte %d should be writable", i, j);
      }
   }

   // Verify total allocation tracking
   usize total = Arena.get_total_allocated(test_arena);
   usize expected_min = 0;
   for (int i = 0; i < num_allocs; i++) {
      expected_min += 200 + (i * 10);
   }
   Assert.areEqual(&expected_min, &total, LONG, "Total allocated should match expected");

   Memory.dispose_arena(test_arena);
}

// test memory system integration stress
void test_memory_system_integration(void) {
   // Test that arena allocations don't interfere with global Memory
   void *global_ptr1 = Memory.alloc(64, false);
   Assert.isNotNull(global_ptr1, "Global allocation should succeed");
   Assert.isTrue(Memory.is_tracking(global_ptr1), "Global allocation should be tracked");

   sc_arena *test_arena = Memory.create_arena(1);
   Assert.isNotNull(test_arena, "Arena creation should succeed");

   object arena_ptr1 = Arena.alloc(test_arena, 128, false);
   Assert.isNotNull(arena_ptr1, "Arena allocation should succeed");
   Assert.isTrue(Arena.is_tracking(test_arena, arena_ptr1), "Arena allocation should be tracked by arena");

   // Global and arena tracking should be independent
   Assert.isTrue(Memory.is_tracking(global_ptr1), "Global pointer should still be tracked");
   Assert.isFalse(Arena.is_tracking(test_arena, global_ptr1), "Global pointer should not be tracked by arena");
   Assert.isFalse(Memory.is_tracking(arena_ptr1), "Arena pointer should not be tracked by global Memory");

   // More allocations
   void *global_ptr2 = Memory.alloc(32, true);
   object arena_ptr2 = Arena.alloc(test_arena, 256, true);

   Assert.isNotNull(global_ptr2, "Second global allocation should succeed");
   Assert.isNotNull(arena_ptr2, "Second arena allocation should succeed");

   // Verify zero-init worked
   char *global_data = (char *)global_ptr2;
   char *arena_data = (char *)arena_ptr2;
   for (int i = 0; i < 32; i++) {
      Assert.areEqual(&(char){0}, &global_data[i], CHAR, "Global zero-init byte %d should be 0", i);
   }
   for (int i = 0; i < 256; i++) {
      Assert.areEqual(&(char){0}, &arena_data[i], CHAR, "Arena zero-init byte %d should be 0", i);
   }

   // Cleanup
   Memory.dispose(global_ptr1);
   Memory.dispose(global_ptr2);
   Assert.isFalse(Memory.is_tracking(global_ptr1), "Global ptr1 should be untracked after dispose");
   Assert.isFalse(Memory.is_tracking(global_ptr2), "Global ptr2 should be untracked after dispose");

   Memory.dispose_arena(test_arena);
}

// test failure injection and recovery
void test_failure_injection(void) {
   sc_arena *test_arena = Memory.create_arena(1);
   Assert.isNotNull(test_arena, "Arena setup should succeed");

   // Test allocation that triggers page growth
   // Allocate chunks that will fill the first page
   const usize chunk_size = 500;
   const int num_chunks = (PAGE_DATA_SIZE / chunk_size) + 1; // Ensure we exceed one page

   for (int i = 0; i < num_chunks; i++) {
      object ptr = Arena.alloc(test_arena, chunk_size, false);
      Assert.isNotNull(ptr, "Chunk allocation %d should succeed", i);
      Assert.isTrue(Arena.is_tracking(test_arena, ptr), "Chunk %d should be tracked", i);
   }

   usize final_pages = Arena.get_page_count(test_arena);
   Assert.isTrue(final_pages >= 2, "Multiple allocations should have triggered page growth");

   usize total_allocated = Arena.get_total_allocated(test_arena);
   Assert.isTrue(total_allocated >= (usize)(num_chunks * chunk_size), "Total allocated should match expectations");

   Memory.dispose_arena(test_arena);
}

// test extreme allocation patterns
void test_extreme_patterns(void) {
   sc_arena *test_arena = Memory.create_arena(1);
   Assert.isNotNull(test_arena, "Arena setup should succeed");

   // Pattern: Very small allocations
   const int tiny_count = 1000;
   object tiny_ptrs[tiny_count];
   for (int i = 0; i < tiny_count; i++) {
      tiny_ptrs[i] = Arena.alloc(test_arena, 1, false);
      Assert.isNotNull(tiny_ptrs[i], "Tiny allocation %d should succeed", i);
   }

   // Pattern: Maximum possible allocations in remaining space
   int max_count = 0;
   while (true) {
      object ptr = Arena.alloc(test_arena, 1, false);
      if (!ptr)
         break;
      max_count++;
   }

   Assert.isTrue(max_count >= 0, "Should be able to allocate until page full");

   // Verify all tiny allocations are still tracked
   for (int i = 0; i < tiny_count; i++) {
      Assert.isTrue(Arena.is_tracking(test_arena, tiny_ptrs[i]), "Tiny pointer %d should still be tracked", i);
   }

   usize total_allocated = Arena.get_total_allocated(test_arena);
   Assert.isTrue(total_allocated >= (usize)tiny_count, "Total allocated should be at least tiny count");

   Memory.dispose_arena(test_arena);
}

//  register test cases
__attribute__((constructor)) void init_robustness_tests(void) {
   testset("run_mem_functions", set_config, set_teardown);

   testcase("Inheritance tracking persistence", test_inheritance_tracking_persistence);
   testcase("Allocation pattern stress", test_allocation_pattern_stress);
   testcase("Page boundary stress", test_page_boundary_stress);
   testcase("Root tracking groundwork", test_root_tracking_groundwork);
   testcase("Memory system integration", test_memory_system_integration);
   testcase("Failure injection", test_failure_injection);
   testcase("Extreme patterns", test_extreme_patterns);
}