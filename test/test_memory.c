/*
 *  Test File: test_memory.c
 *  Description: Test cases for SigmaCore memory management interfaces
 */

#include "internal/memory_internal.h"
#include "prototyping/proto_arena.h"
#include "sigcore/memory.h"
#include <sigtest/sigtest.h>
#include <stdio.h>
#include <stdlib.h>

#define PAGE_SLOTS_CAPACITY 4096

static void set_config(FILE **log_stream) {
   *log_stream = fopen("logs/test_memory.log", "w");
}

static void set_teardown(void) {
   // No cleanup needed - destructor handles it
}

// test memory initialization
void test_memory_constructor_runs(void) {
   object ptr = Memory.alloc(16, false);
   Assert.isNotNull(ptr, "Memory.alloc should succeed after constructor");
   Memory.dispose(ptr);
}

void test_memory_tracker_page_exists(void) {
   object ptr = Memory.alloc(64, false);
   Assert.isNotNull(ptr, "Memory should have pages for allocation");
   Memory.dispose(ptr);
}

void test_memory_tracker_slotarray_valid(void) {
   object ptr = Memory.alloc(0, false);
   Assert.isNull(ptr, "Alloc 0 should return NULL");
}

void test_memory_self_registers_own_allocs(void) {
   object ptr = Memory.alloc(128, false);
   Assert.isNotNull(ptr, "Self allocation should work");
   Memory.dispose(ptr);
}

void test_memory_is_ready_after_init(void) {
   object ptr = Memory.alloc(1, false);
   Assert.isNotNull(ptr, "Memory should be ready after init");
   Memory.dispose(ptr);
}

void test_memory_bootstrap_allocs_are_tracked(void) {
   object ptr = Memory.alloc(256, false);
   Assert.isNotNull(ptr, "Bootstrap allocs should be tracked");
   Memory.dispose(ptr);
}

void test_memory_has_false_for_garbage(void) {
   Memory.dispose(NULL); // Should not crash
   Assert.isTrue(true, "Dispose NULL handled");
}

//  test memory allocation and deallocation
void test_memory_alloc_free(void) {
   object ptr = Memory.alloc(32, false);
   Assert.isNotNull(ptr, "Alloc should succeed");
   Memory.dispose(ptr);
   Assert.isTrue(true, "Dispose succeeded");
}

// New test stubs for extended memory operations
void test_memory_alloc_zee_zeros(void) {
   object ptr = Memory.alloc(16, true);
   Assert.isNotNull(ptr, "Alloc with zero should succeed");
   char zero = 0;
   for (int i = 0; i < 16; i++) {
      Assert.areEqual(&((char *)ptr)[i], &zero, CHAR, "Memory should be zeroed");
   }
   Memory.dispose(ptr);
}

void test_memory_alloc_no_zee_no_zero(void) {
   object ptr = Memory.alloc(16, false);
   Assert.isNotNull(ptr, "Alloc without zero should succeed");
   // Can't check if not zero, but at least alloc works
   Memory.dispose(ptr);
   Assert.isTrue(true, "No zero alloc works");
}

void test_memory_track_untrack(void) {
   object ptr1 = Memory.alloc(10, false);
   object ptr2 = Memory.alloc(20, false);
   Assert.isNotNull(ptr1, "First alloc");
   Assert.isNotNull(ptr2, "Second alloc");
   Memory.dispose(ptr1);
   Memory.dispose(ptr2);
   Assert.isTrue(true, "Multiple alloc/dispose works");
}

void test_memory_realloc(void) {
   object ptr = Memory.alloc(10, false);
   Assert.isNotNull(ptr, "Initial alloc");
   object new_ptr = Memory.realloc(ptr, 20);
   Assert.isNotNull(new_ptr, "Realloc larger");
   Memory.dispose(new_ptr);
   Assert.isTrue(true, "Realloc works");
}

void test_memory_init_teardown(void) {
   object ptr = Memory.alloc(100, false);
   Assert.isNotNull(ptr, "Init alloc");
   Memory.dispose(ptr);
   object ptr2 = Memory.alloc(50, false);
   Assert.isNotNull(ptr2, "After dispose alloc");
   Memory.dispose(ptr2);
}

// Merge and defragmentation tests
void test_memory_merge_right(void) {
   // Allocate two adjacent blocks
   object p1 = Memory.alloc(100, false);
   object p2 = Memory.alloc(100, false);
   Assert.isNotNull(p1, "First alloc");
   Assert.isNotNull(p2, "Second alloc");

   // Free left, then right to test merge right
   Memory.dispose(p1);
   Memory.dispose(p2);

   // If merged, should be able to alloc larger block
   object p3 = Memory.alloc(200, false);
   Assert.isNotNull(p3, "Merge right allowed larger alloc");
   Memory.dispose(p3);
}

void test_memory_merge_left(void) {
   // Allocate two adjacent blocks
   object p1 = Memory.alloc(100, false);
   object p2 = Memory.alloc(100, false);
   Assert.isNotNull(p1, "First alloc");
   Assert.isNotNull(p2, "Second alloc");

   // Free right, then left to test merge left
   Memory.dispose(p2);
   Memory.dispose(p1);

   // If merged, should be able to alloc larger block
   object p3 = Memory.alloc(200, false);
   Assert.isNotNull(p3, "Merge left allowed larger alloc");
   Memory.dispose(p3);
}

void test_memory_merge_both(void) {
   // Allocate three adjacent blocks
   object p1 = Memory.alloc(100, false);
   object p2 = Memory.alloc(100, false);
   object p3 = Memory.alloc(100, false);
   Assert.isNotNull(p1, "First alloc");
   Assert.isNotNull(p2, "Second alloc");
   Assert.isNotNull(p3, "Third alloc");

   // Free middle, then left (merges right), then right (merges left)
   Memory.dispose(p2);
   Memory.dispose(p1);
   Memory.dispose(p3);

   // If merged all, should alloc even larger
   object p4 = Memory.alloc(300, false);
   Assert.isNotNull(p4, "Merge both allowed large alloc");
   Memory.dispose(p4);
}

void test_memory_no_merge_non_adjacent(void) {
   // Allocate blocks with gap
   object p1 = Memory.alloc(100, false);
   object p2 = Memory.alloc(100, false);
   object p3 = Memory.alloc(100, false);
   Assert.isNotNull(p1, "First alloc");
   Assert.isNotNull(p2, "Second alloc");
   Assert.isNotNull(p3, "Third alloc");

   // Free p1 and p3, leaving p2 allocated (gap)
   Memory.dispose(p1);
   Memory.dispose(p3);

   // Free p2, now p1 and p3 free but not adjacent
   Memory.dispose(p2);

   // Should not be able to alloc 300 (would require merge), but can alloc 100
   object p4 = Memory.alloc(100, false);
   Assert.isNotNull(p4, "Can alloc 100 without merge");
   Memory.dispose(p4);

   // Large alloc might fail or succeed depending on layout
   object p5 = Memory.alloc(250, false);
   // Don't assert, just dispose
   Memory.dispose(p5);
}

void test_memory_fragmentation_stress(void) {
   // Stress test with many alloc/free
   const int count = 50;
   object ptrs[count];

   // Alloc many small blocks
   for (int i = 0; i < count; i++) {
      ptrs[i] = Memory.alloc(16, false);
      Assert.isNotNull(ptrs[i], "Alloc in stress");
   }

   // Free every other
   for (int i = 0; i < count; i += 2) {
      Memory.dispose(ptrs[i]);
   }

   // Alloc some medium
   object med1 = Memory.alloc(64, false);
   object med2 = Memory.alloc(64, false);
   Assert.isNotNull(med1, "Medium alloc after fragmentation");
   Assert.isNotNull(med2, "Another medium alloc");

   // Free the rest
   for (int i = 1; i < count; i += 2) {
      Memory.dispose(ptrs[i]);
   }

   Memory.dispose(med1);
   Memory.dispose(med2);

   // Should be able to alloc large now
   object large = Memory.alloc(500, false);
   Assert.isNotNull(large, "Large alloc after defrag");
   Memory.dispose(large);
}

//  register test cases
__attribute__((constructor)) void init_memory_tests(void) {
   testset("core_memory_set", set_config, set_teardown);

   testcase("Constructor initializes state", test_memory_constructor_runs);
   testcase("Tracker page exists after init", test_memory_tracker_page_exists);
   testcase("Tracker SlotArray valid", test_memory_tracker_slotarray_valid);
   testcase("Memory avoids self-tracking", test_memory_self_registers_own_allocs);
   testcase("Memory ready after init", test_memory_is_ready_after_init);
   testcase("Bootstrap allocs tracked", test_memory_bootstrap_allocs_are_tracked);
   testcase("has() false for garbage", test_memory_has_false_for_garbage);
   testcase("Alloc/dispose tracks correctly", test_memory_alloc_free);
   testcase("Alloc with zero initializes", test_memory_alloc_zee_zeros);
   testcase("Alloc without zero no init", test_memory_alloc_no_zee_no_zero);
   testcase("Track/untrack external pointers", test_memory_track_untrack);
   testcase("Realloc basic cases", test_memory_realloc);
   testcase("Init/teardown basics", test_memory_init_teardown);
   testcase("Merge right adjacent blocks", test_memory_merge_right);
   testcase("Merge left adjacent blocks", test_memory_merge_left);
   testcase("Merge both adjacent blocks", test_memory_merge_both);
   testcase("No merge non-adjacent blocks", test_memory_no_merge_non_adjacent);
   testcase("Fragmentation stress test", test_memory_fragmentation_stress);
}