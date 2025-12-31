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
   // Set memory hooks to use sigtest's wrapped functions for tracking
   // Note: sigtest only provides __wrap_malloc and __wrap_free
   Memory.set_alloc_hooks(__wrap_malloc, __wrap_free, NULL, NULL);
}
static void set_teardown(void) {
   Memory.reset_alloc_hooks();
   Memory.teardown();
}

// test memory initialization
void test_memory_constructor_runs(void) {
   // The Memory interface should be initialized by this point
   Assert.isTrue(Memory.is_ready(), "Memory system should be ready after initialization");
}

void test_memory_tracker_page_exists(void) {
   Assert.isTrue(Memory_get_current_page() != NULL, "Current memory page should exist after initialization");
}

void test_memory_tracker_slotarray_valid(void) {
   // Check via allocation tracking
   void *ptr = Memory.alloc(32, false);
   Assert.isTrue(Memory.is_tracking(ptr), "Allocated pointer (size=32) should be tracked by memory system");
   Memory.dispose(ptr);
}

void test_memory_self_registers_own_allocs(void) {
   Assert.isFalse(Memory.is_tracking(Memory_get_current_page()), "Memory system should not track its own internal page structures to avoid self-tracking loops");
}

void test_memory_is_ready_after_init(void) {
   Assert.isTrue(Memory.is_ready(), "Memory.is_ready() should return true after system initialization");
}

void test_memory_bootstrap_allocs_are_tracked(void) {
   // Bootstrap allocations are tracked
   void *ptr = Memory.alloc(64, false);
   Assert.isTrue(Memory.is_tracking(ptr), "Bootstrap allocation (size=64) should be tracked by memory system");
   Memory.dispose(ptr);
}

void test_memory_has_false_for_garbage(void) {
   // is_tracking returns false for invalid pointers
   void *garbage = (void *)0xDEADBEEF;
   Assert.isFalse(Memory.is_tracking(garbage), "Memory.is_tracking() should return false for invalid pointer 0xDEADBEEF");
}

//  test memory allocation and deallocation
void test_memory_alloc_free(void) {
   usize size = 128;
   void *ptr = Memory.alloc(size, false);
   Assert.isTrue(ptr != NULL, "Memory.alloc(size=128, zero_init=false) should succeed");
   Assert.isTrue(Memory.is_tracking(ptr), "Allocated pointer should be tracked by memory system");

   Memory.dispose(ptr);
   Assert.isFalse(Memory.is_tracking(ptr), "Disposed pointer should no longer be tracked");
}

// New test stubs for extended memory operations
void test_memory_alloc_zee_zeros(void) {
   usize size = 100;
   char *ptr = (char *)Memory.alloc(size, true);
   Assert.isTrue(ptr != NULL, "Memory.alloc(size=100, zero_init=true) should succeed and return valid pointer");
   Assert.isTrue(Memory.is_tracking(ptr), "Zero-initialized allocation should be tracked by memory system");
   for (usize i = 0; i < size; i++) {
      Assert.isTrue(ptr[i] == 0, "Zero-initialized memory at index %d should be 0, got %d", i, ptr[i]);
   }
   Memory.dispose(ptr);
   Assert.isFalse(Memory.is_tracking(ptr), "Disposed zero-initialized allocation should no longer be tracked");
}

void test_memory_alloc_no_zee_no_zero(void) {
   usize size = 100;
   char *ptr = (char *)Memory.alloc(size, false);
   Assert.isTrue(ptr != NULL, "Memory.alloc(size=100, zero_init=false) should succeed and return valid pointer");
   Assert.isTrue(Memory.is_tracking(ptr), "Non-zero-initialized allocation should be tracked by memory system");
   // Note: Can't reliably check for non-zero without assuming garbage, but at least it allocates
   Memory.dispose(ptr);
   Assert.isFalse(Memory.is_tracking(ptr), "Disposed non-zero-initialized allocation should no longer be tracked");
}

void test_memory_track_untrack(void) {
   // Test tracking and untracking external pointers
   void *external = malloc(64); // External allocation
   Assert.isFalse(Memory.is_tracking(external), "External malloc() pointer should not be initially tracked by memory system");
   Memory.track(external);
   Assert.isTrue(Memory.is_tracking(external), "External pointer should be tracked after calling Memory.track()");
   Memory.untrack(external);
   Assert.isFalse(Memory.is_tracking(external), "External pointer should no longer be tracked after calling Memory.untrack()");
   free(external);
}

void test_memory_realloc(void) {
   // Test realloc basic cases
   void *ptr = Memory.alloc(32, false);
   Assert.isNotNull(ptr, "Memory.alloc(size=32, zero_init=false) should succeed for realloc test");
   Assert.isTrue(Memory.is_tracking(ptr), "Original allocation should be tracked before realloc");

   // Realloc to larger size
   void *new_ptr = Memory.realloc(ptr, 64);
   Assert.isNotNull(new_ptr, "Memory.realloc() to larger size (64) should succeed");
   Assert.isTrue(Memory.is_tracking(new_ptr), "Reallocated pointer (new size=64) should be tracked");
   Assert.isFalse(Memory.is_tracking(ptr), "Original pointer should no longer be tracked after realloc moved the allocation");

   // Realloc to smaller size
   void *smaller = Memory.realloc(new_ptr, 16);
   Assert.isNotNull(smaller, "Memory.realloc() to smaller size (16) should succeed");
   Assert.isTrue(Memory.is_tracking(smaller), "Reallocated pointer (new size=16) should be tracked");
   Assert.isFalse(Memory.is_tracking(new_ptr), "Previous realloc pointer should no longer be tracked after another realloc");

   // Realloc to zero (dispose)
   void *zero = Memory.realloc(smaller, 0);
   Assert.isNull(zero, "Memory.realloc() to size 0 should return NULL (dispose operation)");
   Assert.isFalse(Memory.is_tracking(smaller), "Pointer should no longer be tracked after realloc to zero (dispose)");

   // Realloc NULL (alloc)
   void *from_null = Memory.realloc(NULL, 128);
   Assert.isNotNull(from_null, "Memory.realloc(NULL, size=128) should allocate new memory");
   Assert.isTrue(Memory.is_tracking(from_null), "Allocation from realloc(NULL, 128) should be tracked");
   Memory.dispose(from_null);
}

void test_memory_init_teardown(void) {
   // Test that init is idempotent
   Assert.isTrue(Memory.is_ready(), "System is ready after constructor");
   Memory.init(); // should do nothing
   Assert.isTrue(Memory.is_ready(), "Memory system should remain ready after redundant init() call");

   // Note: Teardown not tested here as it destroys the system for all tests
}

// Test custom allocation hooks with prototype arena
void test_memory_custom_hooks_basic(void) {
   // Save original hooks
   Memory.reset_alloc_hooks();

   // Set up prototype hooks
   proto_reset_tracking();
   proto_setup_hooks();

   // Test allocation with custom hooks
   void *ptr = Memory.alloc(64, false);
   Assert.isNotNull(ptr, "Memory.alloc() with custom hooks should succeed");
   Assert.areEqual(ptr, PROTO_PTR_1, PTR, "Custom malloc hook should return fixed test pointer");
   Assert.isTrue(proto_verify_allocations(), "Prototype allocation tracking should verify correctly");

   // Test disposal with custom hooks
   Memory.dispose(ptr);

   // Reset to default hooks
   Memory.reset_alloc_hooks();
}

// Test hook reset functionality
void test_memory_hook_reset(void) {
   // Set custom hooks
   proto_reset_tracking();
   proto_setup_hooks();

   // Verify custom hooks are active
   void *ptr1 = Memory.alloc(32, false);
   Assert.areEqual(ptr1, PROTO_PTR_1, PTR, "Custom hook should return PROTO_PTR_1");

   // Dispose with custom hooks still active (since ptr1 is fake)
   Memory.dispose(ptr1);

   // Reset hooks
   Memory.reset_alloc_hooks();

   // Verify default hooks are restored
   void *ptr2 = Memory.alloc(32, false);
   Assert.areNotEqual(ptr2, PROTO_PTR_2, PTR, "After reset, should not return PROTO_PTR_2");
   Assert.isNotNull(ptr2, "Default malloc should still work");

   // Clean up
   Memory.dispose(ptr2);
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
   testcase("Custom allocation hooks", test_memory_custom_hooks_basic);
   testcase("Hook reset functionality", test_memory_hook_reset);
}