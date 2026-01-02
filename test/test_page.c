/*
 *  Test File: test_page.c
 *  Description: Test cases for SigmaCore arena page functionality
 */

#include "internal/page_tests.h"
#include "sigcore/memory.h"
#include "sigcore/slotarray.h"
#include <sigtest/sigtest.h>
#include <stdio.h>
#include <stdlib.h>

#define PAGE_DATA_SIZE 4096

static void set_config(FILE **log_stream) {
   *log_stream = fopen("logs/test_page.log", "w");
   // Set memory hooks to use sigtest's wrapped functions for tracking
   // Memory.set_alloc_hooks(__wrap_malloc, __wrap_free, NULL, NULL);
}
static void set_teardown(void) {
   // Memory.teardown();
}

// test page initialization
void test_page_init(void) {
   sc_page *page = Page_create(PAGE_DATA_SIZE);
   Assert.isNotNull(page, "Page_create() should succeed and return valid page pointer");
   Assert.isNotNull(Page_get_bump(page), "New page should have valid bump pointer");
   usize used = Page_get_used(page);
   Assert.isTrue(used == 0, "New page should have used=0");
   usize capacity = Page_get_capacity(page);
   Assert.isTrue(capacity == PAGE_DATA_SIZE, "Page should have correct capacity");
   // Tracking no longer supported at page level
   // Assert.isNotNull(Page_get_tracked_addrs(page), "Page should have valid tracked_addrs slotarray");
   // usize alloc_count = Page_get_allocation_count(page);
   // Assert.areEqual(&(usize){0}, &alloc_count, LONG, "New page should have no tracked allocations");
   Page_destroy(page);
}

// test page allocation within capacity
void test_page_alloc_within_capacity(void) {
   sc_page *page = Page_create(PAGE_DATA_SIZE);
   Assert.isNotNull(page, "Page setup should succeed");

   usize initial_used = Page_get_used(page);
   usize alloc_size = 128;
   object ptr = Page_alloc(page, alloc_size, false);
   Assert.isNotNull(ptr, "Page_alloc() should succeed for allocation within capacity");
   Assert.isTrue(Page_contains(page, ptr), "Allocated pointer should be within page data area");
   usize final_used = Page_get_used(page);
   usize expected_used = initial_used + alloc_size;
   Assert.isTrue(final_used == expected_used, "Page used should increase by allocation size");
   // Tracking no longer supported
   // usize alloc_count = Page_get_allocation_count(page);
   // Assert.areEqual(&(usize){1}, &alloc_count, LONG, "Page should track one allocation");

   Page_destroy(page);
}

// test page allocation exceeding capacity
void test_page_alloc_exceeds_capacity(void) {
   sc_page *page = Page_create(PAGE_DATA_SIZE);
   Assert.isNotNull(page, "Page setup should succeed");

   // Allocate almost the entire page
   usize large_alloc = PAGE_DATA_SIZE - 64; // Leave some room for slotarray overhead
   object ptr1 = Page_alloc(page, large_alloc, false);
   Assert.isNotNull(ptr1, "Large allocation should succeed");

   // Try to allocate more than remaining space
   usize remaining = Page_get_capacity(page) - Page_get_used(page);
   usize too_big = remaining + 100;
   object ptr2 = Page_alloc(page, too_big, false);
   Assert.isNull(ptr2, "Allocation exceeding page capacity should fail and return NULL");

   Page_destroy(page);
}

// test page zero initialization
void test_page_alloc_zero_init(void) {
   sc_page *page = Page_create(PAGE_DATA_SIZE);
   Assert.isNotNull(page, "Page setup should succeed");

   usize alloc_size = 100;
   char *ptr = (char *)Page_alloc(page, alloc_size, true);
   Assert.isNotNull(ptr, "Zero-initialized allocation should succeed");

   bool all_zero = true;
   for (usize i = 0; i < alloc_size; i++) {
      if (ptr[i] != 0) {
         all_zero = false;
         break;
      }
   }
   Assert.isTrue(all_zero, "Zero-initialized allocation should have all bytes set to 0");

   Page_destroy(page);
}

// test page tracking of allocations (modified: tracking no longer supported)
void test_page_tracking_allocations(void) {
   sc_page *page = Page_create(PAGE_DATA_SIZE);
   Assert.isNotNull(page, "Page setup should succeed");

   // Allocate several pointers
   object ptr1 = Page_alloc(page, 64, false);
   object ptr2 = Page_alloc(page, 32, false);
   object ptr3 = Page_alloc(page, 128, false);
   Assert.isNotNull(ptr1, "First allocation should succeed");
   Assert.isNotNull(ptr2, "Second allocation should succeed");
   Assert.isNotNull(ptr3, "Third allocation should succeed");

   // Tracking no longer supported at page level
   // usize alloc_count = Page_get_allocation_count(page);
   // Assert.areEqual(&(usize){3}, &alloc_count, LONG, "Page should track all three allocations");

   // Verify all pointers are within the page
   Assert.isTrue(Page_contains(page, ptr1), "First pointer should be in page");
   Assert.isTrue(Page_contains(page, ptr2), "Second pointer should be in page");
   Assert.isTrue(Page_contains(page, ptr3), "Third pointer should be in page");

   Page_destroy(page);
}

// test page destruction
void test_page_destroy(void) {
   sc_page *page = Page_create(PAGE_DATA_SIZE);
   Assert.isNotNull(page, "Page setup should succeed");

   // Allocate some memory
   object ptr1 = Page_alloc(page, 64, false);
   object ptr2 = Page_alloc(page, 32, false);
   Assert.isNotNull(ptr1, "Allocation should succeed");
   Assert.isNotNull(ptr2, "Allocation should succeed");

   // Destroy the page - this should free all tracked allocations
   Page_destroy(page);

   // Note: We can't reliably test that the pointers are freed since
   // they were allocated with malloc and freed with free.
   // But we can verify the page destruction doesn't crash.
   Assert.isTrue(true, "Page destruction should complete without errors");
}

// test page allocation at exact capacity boundary
void test_page_alloc_exact_capacity(void) {
   sc_page *page = Page_create(PAGE_DATA_SIZE);
   Assert.isNotNull(page, "Page setup should succeed");

   // Allocate until we can't allocate anymore
   int alloc_count = 0;
   usize total_allocated = 0;
   const usize alloc_size = 64;

   while (true) {
      object ptr = Page_alloc(page, alloc_size, false);
      if (!ptr)
         break;
      alloc_count++;
      total_allocated += alloc_size;
   }

   // Should have allocated something
   Assert.isTrue(alloc_count > 0, "Should be able to allocate at least once");

   // Try to allocate one more byte - should definitely fail now
   object should_fail = Page_alloc(page, 1, false);
   Assert.isNull(should_fail, "Allocation after filling page should fail");

   // Verify our allocations
   usize reported_used = Page_get_used(page);
   // usize reported_count = Page_get_allocation_count(page);
   Assert.isTrue(reported_used == total_allocated, "Reported used should match calculated total");
   // Assert.areEqual(&(usize){alloc_count}, &reported_count, LONG, "Reported count should match actual allocations");
   // Assert.areEqual(&total_allocated, &reported_used, LONG, "Reported used should match calculated total");

   Page_destroy(page);
}

// test page allocation zero bytes
void test_page_alloc_zero_bytes(void) {
   sc_page *page = Page_create(PAGE_DATA_SIZE);
   Assert.isNotNull(page, "Page setup should succeed");

   usize initial_used = Page_get_used(page);
   // usize initial_count = Page_get_allocation_count(page);

   // Allocate zero bytes
   object ptr = Page_alloc(page, 0, false);
   Assert.isNotNull(ptr, "Zero-byte allocation should succeed and return valid pointer");

   usize final_used = Page_get_used(page);
   // usize final_count = Page_get_allocation_count(page);

   // Used should not change
   Assert.isTrue(final_used == initial_used, "Zero-byte allocation should not change used amount");
   // Tracking no longer supported
   // Assert.areEqual(&(usize){initial_count + 1}, &final_count, LONG, "Zero-byte allocation should still be tracked");

   Page_destroy(page);
}

// test page allocation with zero initialization
void test_page_alloc_zero_init_stress(void) {
   sc_page *page = Page_create(PAGE_DATA_SIZE);
   Assert.isNotNull(page, "Page setup should succeed");

   // Allocate multiple zero-initialized blocks
   const int num_allocs = 10;
   object ptrs[10];
   usize sizes[10] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512};

   for (int i = 0; i < num_allocs; i++) {
      ptrs[i] = Page_alloc(page, sizes[i], true);
      Assert.isNotNull(ptrs[i], "Zero-init allocation %d should succeed", i);

      // Verify zero initialization
      char *data = (char *)ptrs[i];
      for (usize j = 0; j < sizes[i]; j++) {
         Assert.areEqual(&(char){0}, &data[j], CHAR, "Byte %d of zero-init allocation %d should be 0", j, i);
      }
   }

   usize total_used = Page_get_used(page);
   usize expected_used = 0;
   for (int i = 0; i < num_allocs; i++)
      expected_used += sizes[i];
   Assert.isTrue(total_used == expected_used, "Total used should match sum of allocations");

   // usize alloc_count = Page_get_allocation_count(page);
   // usize expected_count = num_allocs;
   // Assert.areEqual(&expected_count, &alloc_count, LONG, "Should track all allocations");

   Page_destroy(page);
}

// test page tracking integrity (modified: tracking no longer supported)
void test_page_tracking_integrity(void) {
   sc_page *page = Page_create(PAGE_DATA_SIZE);
   Assert.isNotNull(page, "Page setup should succeed");

   // Allocate various sizes
   const int num_allocs = 20;
   object ptrs[num_allocs];
   for (int i = 0; i < num_allocs; i++) {
      usize size = 10 + (i * 5); // 10, 15, 20, ..., 105
      ptrs[i] = Page_alloc(page, size, false);
      Assert.isNotNull(ptrs[i], "Allocation %d should succeed", i);
   }

   // Verify all pointers are within page bounds
   for (int i = 0; i < num_allocs; i++) {
      Assert.isTrue(Page_contains(page, ptrs[i]), "Pointer %d should be within page bounds", i);
   }

   // Tracking no longer supported
   // slotarray tracker = Page_get_tracked_addrs(page);
   // usize cap = SlotArray.capacity(tracker);
   // usize found_count = 0;
   // ... (removed tracking checks)

   Page_destroy(page);
}

// test page memory exhaustion simulation
void test_page_memory_exhaustion(void) {
   sc_page *page = Page_create(PAGE_DATA_SIZE);
   Assert.isNotNull(page, "Page setup should succeed");

   // Keep allocating until we can't anymore
   int alloc_count = 0;
   usize total_allocated = 0;
   const usize alloc_size = 64;

   while (true) {
      object ptr = Page_alloc(page, alloc_size, false);
      if (!ptr)
         break;

      Assert.isTrue(Page_contains(page, ptr), "Allocation %d should be within page", alloc_count);
      alloc_count++;
      total_allocated += alloc_size;
   }

   // Should have allocated something
   Assert.isTrue(alloc_count > 0, "Should be able to allocate at least once");

   usize reported_used = Page_get_used(page);
   // usize reported_count = Page_get_allocation_count(page);

   Assert.isTrue(reported_used == total_allocated, "Reported used should match calculated total");
   // Assert.areEqual(&(usize){alloc_count}, &reported_count, LONG, "Reported count should match actual allocations");

   // Tracking no longer supported
   // slotarray tracker = Page_get_tracked_addrs(page);
   // ... (removed tracking verification)

   Page_destroy(page);
}

// test page null handling
void test_page_null_safety(void) {
   // Test null page operations
   Assert.isNull(Page_get_bump(NULL), "Page_get_bump(NULL) should return NULL");
   usize used_null = Page_get_used(NULL);
   usize expected_zero = 0;
   Assert.isTrue(used_null == expected_zero, "Page_get_used(NULL) should return 0");
   usize capacity_null = Page_get_capacity(NULL);
   usize expected_capacity = PAGE_DATA_SIZE;
   Assert.isTrue(capacity_null == expected_capacity, "Page_get_capacity(NULL) should return PAGE_DATA_SIZE");
   Assert.isNull(Page_get_tracked_addrs(NULL), "Page_get_tracked_addrs(NULL) should return NULL");
   // usize alloc_count_null = Page_get_allocation_count(NULL);
   // Assert.areEqual(&expected_zero, &alloc_count_null, LONG, "Page_get_allocation_count(NULL) should return 0");
   Assert.isFalse(Page_contains(NULL, (object)0x1000), "Page_contains(NULL, ptr) should return false");

   // Test allocation on null page
   Assert.isNull(Page_alloc(NULL, 64, false), "Page_alloc(NULL, ...) should return NULL");

   // Test destruction of null page (should not crash)
   Page_destroy(NULL);
}

// test page allocation size extremes
void test_page_alloc_size_extremes(void) {
   sc_page *page = Page_create(PAGE_DATA_SIZE);
   Assert.isNotNull(page, "Page setup should succeed");

   // Test very small allocations
   object tiny_ptr = Page_alloc(page, 1, false);
   Assert.isNotNull(tiny_ptr, "1-byte allocation should succeed");
   Assert.isTrue(Page_contains(page, tiny_ptr), "Tiny allocation should be within page");

   // Test allocation that would exceed page capacity
   usize remaining = PAGE_DATA_SIZE - Page_get_used(page);
   object too_big = Page_alloc(page, remaining + 1, false);
   Assert.isNull(too_big, "Allocation exceeding remaining capacity should fail");

   // Test maximum possible allocation in remaining space
   object max_ptr = Page_alloc(page, remaining, false);
   Assert.isNotNull(max_ptr, "Maximum allocation should succeed");
   Assert.isTrue(Page_contains(page, max_ptr), "Maximum allocation should be within page");

   // Verify that page is now full (next allocation should fail)
   object should_fail = Page_alloc(page, 1, false);
   Assert.isNull(should_fail, "Allocation after filling page should fail");

   Page_destroy(page);
}

//  register test cases
__attribute__((constructor)) void init_page_tests(void) {
   testset("core_page_set", set_config, set_teardown);

   testcase("Page initialization", test_page_init);
   testcase("Allocation within capacity", test_page_alloc_within_capacity);
   testcase("Allocation exceeds capacity", test_page_alloc_exceeds_capacity);
   testcase("Zero initialization", test_page_alloc_zero_init);
   testcase("Tracking allocations", test_page_tracking_allocations);
   testcase("Page destruction", test_page_destroy);
   testcase("Exact capacity boundary", test_page_alloc_exact_capacity);
   testcase("Zero byte allocation", test_page_alloc_zero_bytes);
   testcase("Zero init stress test", test_page_alloc_zero_init_stress);
   testcase("Tracking integrity", test_page_tracking_integrity);
   testcase("Memory exhaustion", test_page_memory_exhaustion);
   testcase("Null safety", test_page_null_safety);
   testcase("Size extremes", test_page_alloc_size_extremes);
}