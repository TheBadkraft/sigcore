/*
 *  Test File: test_arena.c
 *  Description: Test cases for SigmaCore arena functionality
 */

#include "sigcore/arena.h"
#include "sigcore/memory.h"
#include <sigtest/sigtest.h>
#include <stdio.h>
#include <stdlib.h>

static void set_config(FILE **log_stream) {
   *log_stream = fopen("logs/test_arena.log", "w");
   // Set memory hooks to use sigtest's wrapped functions for tracking
   Memory.set_alloc_hooks(__wrap_malloc, __wrap_free, NULL, NULL);
}
static void set_teardown(void) {
   Memory.reset_alloc_hooks();
   Memory.teardown();
}

// test arena creation
void test_arena_create(void) {
   sc_arena *test_arena = Memory.create_arena(1);
   Assert.isNotNull(test_arena, "Memory.create_arena(1) should succeed and return valid arena pointer");
   usize expected_pages = 1;
   usize actual_pages = Arena.get_page_count(test_arena);
   Assert.areEqual(&expected_pages, &actual_pages, LONG, "Arena should have 1 page initially");
   usize expected_alloc = 0;
   usize actual_alloc = Arena.get_total_allocated(test_arena);
   Assert.areEqual(&expected_alloc, &actual_alloc, LONG, "New arena should have no allocated bytes");
   Memory.dispose_arena(test_arena);
}

// test arena allocation
void test_arena_alloc(void) {
   sc_arena *test_arena = Memory.create_arena(1);
   Assert.isNotNull(test_arena, "Arena setup should succeed");

   usize alloc_size = 128;
   object ptr = Arena.alloc(test_arena, alloc_size, false);
   Assert.isNotNull(ptr, "Arena.alloc() should succeed for allocation");
   Assert.isTrue(Arena.is_tracking(test_arena, ptr), "Allocated pointer should be tracked by arena");
   usize actual_alloc = Arena.get_total_allocated(test_arena);
   Assert.areEqual(&alloc_size, &actual_alloc, LONG, "Arena should report correct total allocated bytes");

   Memory.dispose_arena(test_arena);
}

// test arena zero initialization
void test_arena_alloc_zero(void) {
   sc_arena *test_arena = Memory.create_arena(1);
   Assert.isNotNull(test_arena, "Arena setup should succeed");

   usize alloc_size = 100;
   char *ptr = (char *)Arena.alloc(test_arena, alloc_size, true);
   Assert.isNotNull(ptr, "Zero-initialized allocation should succeed");

   bool all_zero = true;
   for (usize i = 0; i < alloc_size; i++) {
      if (ptr[i] != 0) {
         all_zero = false;
         break;
      }
   }
   Assert.isTrue(all_zero, "Zero-initialized allocation should have all bytes set to 0");

   Memory.dispose_arena(test_arena);
}

// test arena multi-page allocation
void test_arena_multi_page(void) {
   sc_arena *test_arena = Memory.create_arena(1);
   Assert.isNotNull(test_arena, "Arena setup should succeed");

   // Allocate enough to potentially span multiple pages
   usize total_alloc = 0;
   for (int i = 0; i < 10; i++) {
      usize size = 100 + (i * 50); // Varying sizes
      object ptr = Arena.alloc(test_arena, size, false);
      Assert.isNotNull(ptr, "Allocation %d should succeed", i);
      Assert.isTrue(Arena.is_tracking(test_arena, ptr), "Pointer %d should be tracked", i);
      total_alloc += size;
   }

   usize actual_total = Arena.get_total_allocated(test_arena);
   Assert.areEqual(&total_alloc, &actual_total, LONG, "Arena should report correct total allocated bytes");
   usize page_count = Arena.get_page_count(test_arena);
   Assert.isTrue(page_count >= 1, "Arena should have at least 1 page (may have grown to %d)", page_count);

   Memory.dispose_arena(test_arena);
}

// test arena tracking
void test_arena_tracking(void) {
   sc_arena *test_arena = Memory.create_arena(1);
   Assert.isNotNull(test_arena, "Arena setup should succeed");

   // Allocate several pointers
   object ptr1 = Arena.alloc(test_arena, 64, false);
   object ptr2 = Arena.alloc(test_arena, 32, false);
   object ptr3 = Arena.alloc(test_arena, 128, false);
   Assert.isNotNull(ptr1, "First allocation should succeed");
   Assert.isNotNull(ptr2, "Second allocation should succeed");
   Assert.isNotNull(ptr3, "Third allocation should succeed");

   // Verify tracking
   Assert.isTrue(Arena.is_tracking(test_arena, ptr1), "ptr1 should be tracked");
   Assert.isTrue(Arena.is_tracking(test_arena, ptr2), "ptr2 should be tracked");
   Assert.isTrue(Arena.is_tracking(test_arena, ptr3), "ptr3 should be tracked");

   // Test non-tracked pointer
   void *external = malloc(64);
   Assert.isFalse(Arena.is_tracking(test_arena, external), "External pointer should not be tracked by arena");
   free(external);

   Memory.dispose_arena(test_arena);
}

// test arena destruction
void test_arena_destroy(void) {
   sc_arena *test_arena = Memory.create_arena(2);
   Assert.isNotNull(test_arena, "Arena setup should succeed");

   // Allocate some memory
   object ptr1 = Arena.alloc(test_arena, 64, false);
   object ptr2 = Arena.alloc(test_arena, 32, false);
   Assert.isNotNull(ptr1, "Allocation should succeed");
   Assert.isNotNull(ptr2, "Allocation should succeed");

   // Destroy the arena - this should free all resources
   Memory.dispose_arena(test_arena);

   // Note: We can't reliably test that the pointers are freed since
   // they were bump-allocated from page data areas.
   // But we can verify the arena destruction doesn't crash.
   Assert.isTrue(true, "Arena destruction should complete without errors");
}

// test arena stress allocation
void test_arena_stress_alloc(void) {
   sc_arena *test_arena = Memory.create_arena(1);
   Assert.isNotNull(test_arena, "Arena setup should succeed");

   // Perform many allocations of varying sizes
   const int num_allocs = 100;
   object ptrs[num_allocs];
   usize total_expected = 0;

   for (int i = 0; i < num_allocs; i++) {
      usize size = 10 + (i % 50); // Cycle through sizes 10-59
      ptrs[i] = Arena.alloc(test_arena, size, false);
      Assert.isNotNull(ptrs[i], "Allocation %d should succeed", i);
      Assert.isTrue(Arena.is_tracking(test_arena, ptrs[i]), "Allocation %d should be tracked", i);
      total_expected += size;
   }

   usize total_actual = Arena.get_total_allocated(test_arena);
   Assert.areEqual(&total_expected, &total_actual, LONG, "Total allocated should match expected sum");

   usize page_count = Arena.get_page_count(test_arena);
   Assert.isTrue(page_count >= 1, "Should have at least 1 page (actual: %d)", page_count);

   // Verify all pointers are still tracked
   for (int i = 0; i < num_allocs; i++) {
      Assert.isTrue(Arena.is_tracking(test_arena, ptrs[i]), "Pointer %d should still be tracked after all allocations", i);
   }

   Memory.dispose_arena(test_arena);
}

// test arena page growth
void test_arena_page_growth(void) {
   sc_arena *test_arena = Memory.create_arena(1);
   Assert.isNotNull(test_arena, "Arena setup should succeed");

   usize initial_pages = Arena.get_page_count(test_arena);
   Assert.areEqual(&(usize){1}, &initial_pages, LONG, "Should start with 1 page");

   // Allocate enough to force multiple page creations
   usize large_alloc_size = 2000; // Larger than single page capacity
   object ptr1 = Arena.alloc(test_arena, large_alloc_size, false);
   Assert.isNotNull(ptr1, "Large allocation should succeed");

   usize pages_after_large = Arena.get_page_count(test_arena);
   Assert.isTrue(pages_after_large >= initial_pages, "Page count should not decrease");

   // Allocate many small ones to potentially trigger more page growth
   const int small_allocs = 50;
   for (int i = 0; i < small_allocs; i++) {
      object ptr = Arena.alloc(test_arena, 100, false);
      Assert.isNotNull(ptr, "Small allocation %d should succeed", i);
   }

   usize final_pages = Arena.get_page_count(test_arena);
   usize final_allocated = Arena.get_total_allocated(test_arena);
   usize expected_min_allocated = large_alloc_size + (small_allocs * 100);

   Assert.isTrue(final_pages >= pages_after_large, "Page count should be stable or increase");
   Assert.isTrue(final_allocated >= expected_min_allocated, "Total allocated should be at least expected minimum");

   Memory.dispose_arena(test_arena);
}

// test arena null safety
void test_arena_null_safety(void) {
   // Test null arena operations
   Assert.isNull(Arena.alloc(NULL, 64, false), "Arena.alloc(NULL, ...) should return NULL");
   Assert.isFalse(Arena.is_tracking(NULL, (object)0x1000), "Arena.is_tracking(NULL, ptr) should return false");
   usize page_count_null = Arena.get_page_count(NULL);
   usize expected_zero = 0;
   Assert.areEqual(&expected_zero, &page_count_null, LONG, "Arena.get_page_count(NULL) should return 0");
   usize total_null = Arena.get_total_allocated(NULL);
   Assert.areEqual(&expected_zero, &total_null, LONG, "Arena.get_total_allocated(NULL) should return 0");

   // Test destruction of null arena (should not crash)
   Memory.dispose_arena(NULL);
}

// test arena allocation failures
void test_arena_alloc_failures(void) {
   sc_arena *test_arena = Memory.create_arena(1);
   Assert.isNotNull(test_arena, "Arena setup should succeed");

   // Test allocation of 1 byte
   object zero_ptr = Arena.alloc(test_arena, 1, false);
   Assert.isNotNull(zero_ptr, "1-byte allocation should succeed");
   Assert.isTrue(Arena.is_tracking(test_arena, zero_ptr), "1-byte allocation should be tracked");

   // Test allocation with zero init
   object zero_init_ptr = Arena.alloc(test_arena, 1, true);
   Assert.isNotNull(zero_init_ptr, "1-byte zero-init allocation should succeed");
   Assert.isTrue(Arena.is_tracking(test_arena, zero_init_ptr), "1-byte zero-init allocation should be tracked");

   // Test very large allocation that exceeds page capacity
   // Note: Bump allocation can't handle allocations larger than page size
   object large_ptr = Arena.alloc(test_arena, 10000, false); // Much larger than page size
   Assert.isNull(large_ptr, "Very large allocation should fail (exceeds page capacity)");

   Memory.dispose_arena(test_arena);
}

// test arena tracking accuracy
void test_arena_tracking_accuracy(void) {
   sc_arena *test_arena = Memory.create_arena(1);
   Assert.isNotNull(test_arena, "Arena setup should succeed");

   // Allocate several pointers
   const int num_ptrs = 10;
   object ptrs[10];
   usize sizes[10] = {16, 32, 64, 128, 256, 512, 1024, 2048, 100, 50};

   usize total_expected = 0;
   for (int i = 0; i < num_ptrs; i++) {
      ptrs[i] = Arena.alloc(test_arena, sizes[i], false);
      Assert.isNotNull(ptrs[i], "Allocation %d should succeed", i);
      total_expected += sizes[i];
   }

   // Verify tracking for all pointers
   for (int i = 0; i < num_ptrs; i++) {
      Assert.isTrue(Arena.is_tracking(test_arena, ptrs[i]), "Pointer %d should be tracked", i);
   }

   // Test non-tracked pointers
   void *external = malloc(64);
   Assert.isFalse(Arena.is_tracking(test_arena, external), "External malloc pointer should not be tracked");
   free(external);

   void *garbage = (void *)0xDEADBEEF;
   Assert.isFalse(Arena.is_tracking(test_arena, garbage), "Garbage pointer should not be tracked");

   // Verify total allocated
   usize total_actual = Arena.get_total_allocated(test_arena);
   Assert.areEqual(&total_expected, &total_actual, LONG, "Total allocated should match expected");

   Memory.dispose_arena(test_arena);
}

// test arena mixed allocation patterns
void test_arena_mixed_patterns(void) {
   sc_arena *test_arena = Memory.create_arena(1);
   Assert.isNotNull(test_arena, "Arena setup should succeed");

   // Mix of zero-init and non-zero-init allocations
   object ptr1 = Arena.alloc(test_arena, 100, true);  // zero-init
   object ptr2 = Arena.alloc(test_arena, 200, false); // no zero-init
   object ptr3 = Arena.alloc(test_arena, 50, true);   // zero-init
   object ptr4 = Arena.alloc(test_arena, 75, false);  // no zero-init

   Assert.isNotNull(ptr1, "Zero-init allocation should succeed");
   Assert.isNotNull(ptr2, "Non-zero-init allocation should succeed");
   Assert.isNotNull(ptr3, "Zero-init allocation should succeed");
   Assert.isNotNull(ptr4, "Non-zero-init allocation should succeed");

   // Verify zero-init worked for ptr1 and ptr3
   char *data1 = (char *)ptr1;
   char *data3 = (char *)ptr3;
   for (usize i = 0; i < 100; i++) {
      Assert.areEqual(&(char){0}, &data1[i], CHAR, "Zero-init ptr1 byte %d should be 0", i);
   }
   for (usize i = 0; i < 50; i++) {
      Assert.areEqual(&(char){0}, &data3[i], CHAR, "Zero-init ptr3 byte %d should be 0", i);
   }

   // All should be tracked
   Assert.isTrue(Arena.is_tracking(test_arena, ptr1), "ptr1 should be tracked");
   Assert.isTrue(Arena.is_tracking(test_arena, ptr2), "ptr2 should be tracked");
   Assert.isTrue(Arena.is_tracking(test_arena, ptr3), "ptr3 should be tracked");
   Assert.isTrue(Arena.is_tracking(test_arena, ptr4), "ptr4 should be tracked");

   usize expected_total = 100 + 200 + 50 + 75;
   usize actual_total = Arena.get_total_allocated(test_arena);
   Assert.areEqual(&expected_total, &actual_total, LONG, "Total allocated should be correct");

   Memory.dispose_arena(test_arena);
}

// test arena creation with different initial page counts
void test_arena_initial_pages(void) {
   // Test with 0 initial pages
   sc_arena *arena0 = Memory.create_arena(0);
   Assert.isNotNull(arena0, "Arena with 0 initial pages should succeed");
   usize pages0 = Arena.get_page_count(arena0);
   Assert.areEqual(&(usize){0}, &pages0, LONG, "Arena with 0 initial pages should have 0 pages");

   // First allocation should create a page
   object ptr = Arena.alloc(arena0, 64, false);
   Assert.isNotNull(ptr, "First allocation on 0-page arena should succeed");
   usize pages0_after = Arena.get_page_count(arena0);
   Assert.areEqual(&(usize){1}, &pages0_after, LONG, "First allocation should create 1 page");
   Memory.dispose_arena(arena0);

   // Test with multiple initial pages
   sc_arena *arena5 = Memory.create_arena(5);
   Assert.isNotNull(arena5, "Arena with 5 initial pages should succeed");
   usize pages5 = Arena.get_page_count(arena5);
   Assert.areEqual(&(usize){5}, &pages5, LONG, "Arena should have 5 initial pages");
   Memory.dispose_arena(arena5);
}

// test frame begin/end basic functionality
void test_frame_basic(void) {
   sc_arena *arena = Memory.create_arena(1);
   Assert.isNotNull(arena, "Arena setup should succeed");

   // Begin a frame
   frame test_frame = Arena.begin_frame(arena);
   Assert.isNotNull(test_frame, "Arena.begin_frame() should succeed");

   // Allocate some memory in the frame
   object ptr1 = Arena.alloc(arena, 64, false);
   Assert.isNotNull(ptr1, "Allocation in frame should succeed");
   Assert.isTrue(Arena.is_tracking(arena, ptr1), "Frame allocation should be tracked");

   object ptr2 = Arena.alloc(arena, 128, true);
   Assert.isNotNull(ptr2, "Second allocation in frame should succeed");
   Assert.isTrue(Arena.is_tracking(arena, ptr2), "Second frame allocation should be tracked");

   // Check that memory is allocated
   usize alloc_before = Arena.get_total_allocated(arena);
   Assert.isTrue(alloc_before >= 192, "Should have allocated at least 192 bytes");

   // End the frame - should free the allocations
   Arena.end_frame(test_frame);

   // After ending frame, allocations should be freed (bump pointer reset)
   // Note: is_tracking may still return true for the freed memory regions
   // but the memory is effectively freed for reuse

   usize alloc_after = Arena.get_total_allocated(arena);
   Assert.isTrue(alloc_after < alloc_before, "Total allocated should decrease after frame end");

   // Should be able to allocate new memory in the same space
   object ptr3 = Arena.alloc(arena, 64, false);
   Assert.isNotNull(ptr3, "Allocation after frame end should succeed");

   Memory.dispose_arena(arena);
}

// test nested frames
void test_frame_nested(void) {
   sc_arena *arena = Memory.create_arena(1);
   Assert.isNotNull(arena, "Arena setup should succeed");

   // Begin outer frame
   frame outer_frame = Arena.begin_frame(arena);
   Assert.isNotNull(outer_frame, "Outer frame creation should succeed");

   object outer_ptr = Arena.alloc(arena, 100, false);
   Assert.isNotNull(outer_ptr, "Outer frame allocation should succeed");

   // Begin inner frame
   frame inner_frame = Arena.begin_frame(arena);
   Assert.isNotNull(inner_frame, "Inner frame creation should succeed");

   object inner_ptr = Arena.alloc(arena, 50, false);
   Assert.isNotNull(inner_ptr, "Inner frame allocation should succeed");

   usize alloc_with_both = Arena.get_total_allocated(arena);

   // End inner frame first
   Arena.end_frame(inner_frame);

   usize alloc_after_inner = Arena.get_total_allocated(arena);
   Assert.isTrue(alloc_after_inner < alloc_with_both, "Allocation should decrease after inner frame end");

   // Outer allocation should still be there
   Assert.isTrue(Arena.is_tracking(arena, outer_ptr), "Outer allocation should still be tracked");

   // End outer frame
   Arena.end_frame(outer_frame);

   usize alloc_final = Arena.get_total_allocated(arena);
   Assert.isTrue(alloc_final < alloc_after_inner, "Allocation should decrease after outer frame end");

   Memory.dispose_arena(arena);
}

// test frame with page growth
void test_frame_page_growth(void) {
   sc_arena *arena = Memory.create_arena(1);
   Assert.isNotNull(arena, "Arena setup should succeed");

   // Begin frame
   frame test_frame = Arena.begin_frame(arena);
   Assert.isNotNull(test_frame, "Frame creation should succeed");

   usize initial_pages = Arena.get_page_count(arena);

   // Allocate enough to force page growth
   const usize alloc_count = 100;
   const usize alloc_size = 1024; // Large allocations to force page growth

   object *ptrs = malloc(sizeof(object) * alloc_count);
   Assert.isNotNull(ptrs, "Test allocation array should succeed");

   for (usize i = 0; i < alloc_count; i++) {
      ptrs[i] = Arena.alloc(arena, alloc_size, false);
      Assert.isNotNull(ptrs[i], "Large allocation %zu should succeed", i);
   }

   usize pages_after_alloc = Arena.get_page_count(arena);
   Assert.isTrue(pages_after_alloc > initial_pages, "Should have grown pages during frame");

   usize alloc_peak = Arena.get_total_allocated(arena);
   Assert.isTrue(alloc_peak >= alloc_count * alloc_size, "Should have allocated significant memory");

   // End frame - should reset bump pointers
   Arena.end_frame(test_frame);

   usize alloc_after_frame = Arena.get_total_allocated(arena);
   Assert.isTrue(alloc_after_frame < alloc_peak, "Allocation should decrease significantly after frame end");

   // Pages should still exist but bump pointers should be reset
   usize pages_final = Arena.get_page_count(arena);
   Assert.areEqual(&pages_after_alloc, &pages_final, LONG, "Page count should remain the same");

   free(ptrs);
   Memory.dispose_arena(arena);
}

// test frame edge cases
void test_frame_edge_cases(void) {
   sc_arena *arena = Memory.create_arena(1);
   Assert.isNotNull(arena, "Arena setup should succeed");

   // Test begin_frame on NULL arena
   frame null_frame = Arena.begin_frame(NULL);
   Assert.isNull(null_frame, "begin_frame(NULL) should return NULL");

   // Test end_frame on NULL frame
   Arena.end_frame(NULL); // Should not crash

   // Test empty frame (no allocations)
   frame empty_frame = Arena.begin_frame(arena);
   Assert.isNotNull(empty_frame, "Empty frame creation should succeed");

   usize alloc_before = Arena.get_total_allocated(arena);
   Arena.end_frame(empty_frame);
   usize alloc_after = Arena.get_total_allocated(arena);
   Assert.areEqual(&alloc_before, &alloc_after, LONG, "Empty frame should not change allocation count");

   // Test frame with zero-sized allocation
   frame zero_frame = Arena.begin_frame(arena);
   object zero_ptr = Arena.alloc(arena, 0, false);
   Assert.isNotNull(zero_ptr, "Zero-sized allocation should succeed");

   Arena.end_frame(zero_frame);

   Memory.dispose_arena(arena);
}

// test frame early exit (outer ended before inner)
void test_frame_early_exit(void) {
   sc_arena *arena = Memory.create_arena(1);
   Assert.isNotNull(arena, "Arena setup should succeed");

   // Begin outer frame
   frame outer_frame = Arena.begin_frame(arena);
   Assert.isNotNull(outer_frame, "Outer frame creation should succeed");

   object outer_ptr = Arena.alloc(arena, 100, false);
   Assert.isNotNull(outer_ptr, "Outer allocation should succeed");

   // Begin inner frame
   frame inner_frame = Arena.begin_frame(arena);
   Assert.isNotNull(inner_frame, "Inner frame creation should succeed");

   object inner_ptr = Arena.alloc(arena, 50, false);
   Assert.isNotNull(inner_ptr, "Inner allocation should succeed");

   usize alloc_with_both = Arena.get_total_allocated(arena);
   Assert.isTrue(alloc_with_both >= 150, "Should have allocated at least 150 bytes");

   // Simulate early exit: end outer frame before inner frame
   // This should automatically clean up the inner frame too
   Arena.end_frame(outer_frame);

   usize alloc_after_early_exit = Arena.get_total_allocated(arena);
   Assert.isTrue(alloc_after_early_exit < alloc_with_both, "Early exit should free all frame allocations");

   // Verify that both frames are cleaned up
   // The inner frame handle should now be invalid, but ending it again should be safe
   Arena.end_frame(inner_frame); // Should not crash or double-free

   usize alloc_final = Arena.get_total_allocated(arena);
   Assert.areEqual(&alloc_after_early_exit, &alloc_final, LONG, "Ending already-cleaned inner frame should not change allocation");

   Memory.dispose_arena(arena);
}

//  register test cases
__attribute__((constructor)) void init_arena_tests(void) {
   testset("core_arena_set", set_config, set_teardown);

   testcase("Arena creation", test_arena_create);
   testcase("Arena allocation", test_arena_alloc);
   testcase("Arena zero initialization", test_arena_alloc_zero);
   testcase("Arena multi-page allocation", test_arena_multi_page);
   testcase("Arena tracking", test_arena_tracking);
   testcase("Arena destruction", test_arena_destroy);
   testcase("Arena stress allocation", test_arena_stress_alloc);
   testcase("Arena page growth", test_arena_page_growth);
   testcase("Arena null safety", test_arena_null_safety);
   testcase("Arena allocation failures", test_arena_alloc_failures);
   testcase("Arena tracking accuracy", test_arena_tracking_accuracy);
   testcase("Arena mixed patterns", test_arena_mixed_patterns);
   testcase("Arena initial pages", test_arena_initial_pages);
   testcase("Frame basic functionality", test_frame_basic);
   testcase("Frame nested frames", test_frame_nested);
   testcase("Frame with page growth", test_frame_page_growth);
   testcase("Frame edge cases", test_frame_edge_cases);
   testcase("Frame early exit handling", test_frame_early_exit);
}