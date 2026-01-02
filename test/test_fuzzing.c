/*
 *  Test File: test_fuzzing.c
 *  Description: Fuzzing tests for SigmaCore components with size_t boundary pressure
 *               Tests undefined behavior resistance with extreme size_t values
 */

#include "sigcore/arena.h"
#include "sigcore/farray.h"
#include "sigcore/list.h"
#include "sigcore/memory.h"
#include "sigcore/parray.h"
#include "sigcore/slotarray.h"
#include <sigtest/fuzzing.h>
#include <sigtest/sigtest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void set_config(FILE **log_stream) {
   *log_stream = fopen("logs/test_fuzzing.log", "w");
   // Memory hooks removed - using default allocation
}

static void set_teardown(void) {
   // No teardown needed
}

// test arena allocation fuzzing with size_t boundaries
void test_arena_allocation_fuzz(void *param) {
   usize size = *(usize *)param;

   sc_arena *test_arena = Memory.Arena.create(1);
   Assert.isNotNull(test_arena, "Arena setup should succeed");

   // Try allocation - expect small sizes to succeed, large sizes to fail
   object ptr = Arena.alloc(test_arena, size, false);

   if (size <= 4096) { // PAGE_DATA_SIZE
      // Small allocations should succeed
      Assert.isNotNull(ptr, "Arena allocation of size %zu should succeed", size);
      if (ptr) {
         Assert.isTrue(Arena.is_tracking(test_arena, ptr), "Allocated pointer should be tracked");
      }
   } else {
      // Large allocations should fail (bump allocation limit)
      Assert.isNull(ptr, "Arena allocation of size %zu should fail (exceeds page capacity)", size);
   }

   Memory.Arena.dispose(test_arena);
}

// test farray fuzzing with size_t boundaries
void test_farray_fuzz(void *param) {
   usize capacity = *(usize *)param;

   farray arr = FArray.new(capacity, sizeof(int));

   if (capacity <= 1000000 && capacity != SIZE_MAX && capacity != SIZE_MAX - 1) { // Reasonable limit for testing, avoid extreme values
      Assert.isNotNull(arr, "FArray creation with capacity %zu should succeed", capacity);
      if (arr) {
         int actual_capacity = FArray.capacity(arr, sizeof(int));
         Assert.areEqual(&capacity, &(size_t){actual_capacity}, LONG, "FArray should have correct capacity");
         FArray.dispose(arr);
      }
   } else {
      // Very large capacities should fail
      Assert.isNull(arr, "FArray creation with capacity %zu should fail", capacity);
   }
}

// test list fuzzing with size_t boundaries
void test_list_fuzz(void *param) {
   usize capacity = *(usize *)param;

   list lst = List.new(capacity, sizeof(usize));

   if (capacity <= 100000 && capacity != SIZE_MAX && capacity != SIZE_MAX - 1) { // Reasonable limit for testing, avoid extreme values
      Assert.isNotNull(lst, "List creation with capacity %zu should succeed", capacity);
      if (lst) {
         usize actual_capacity = List.capacity(lst);
         Assert.areEqual(&capacity, &actual_capacity, LONG, "List should have correct capacity");
         List.dispose(lst);
      }
   } else {
      // Very large capacities should fail
      Assert.isNull(lst, "List creation with capacity %zu should fail", capacity);
   }
}

// test parray fuzzing with size_t boundaries
void test_parray_fuzz(void *param) {
   usize capacity = *(usize *)param;

   parray arr = PArray.new(capacity);

   if (capacity <= 100000 && capacity != SIZE_MAX && capacity != SIZE_MAX - 1) { // Reasonable limit for testing, avoid extreme values
      Assert.isNotNull(arr, "PArray creation with capacity %zu should succeed", capacity);
      if (arr) {
         int actual_capacity = PArray.capacity(arr);
         Assert.areEqual(&capacity, &(size_t){actual_capacity}, LONG, "PArray should have correct capacity");
         PArray.dispose(arr);
      }
   } else {
      // Very large capacities should fail
      Assert.isNull(arr, "PArray creation with capacity %zu should fail", capacity);
   }
}

// test slotarray fuzzing with size_t boundaries
void test_slotarray_fuzz(void *param) {
   usize capacity = *(usize *)param;

   slotarray arr = SlotArray.new(capacity);

   if (capacity <= 100000 && capacity != SIZE_MAX && capacity != SIZE_MAX - 1) { // Reasonable limit for testing, avoid extreme values
      Assert.isNotNull(arr, "SlotArray creation with capacity %zu should succeed", capacity);
      if (arr) {
         usize actual_capacity = SlotArray.capacity(arr);
         Assert.areEqual(&capacity, &actual_capacity, LONG, "SlotArray should have correct capacity");
         SlotArray.dispose(arr);
      }
   } else {
      // Very large capacities should fail
      Assert.isNull(arr, "SlotArray creation with capacity %zu should fail", capacity);
   }
}

// test memory allocation fuzzing (similar to the example)
void test_memory_allocation_fuzz(void *param) {
   usize size = *(usize *)param;

   object ptr = Memory.alloc(size, false);

   if (size > 0 && size <= 1000000 && size != SIZE_MAX && size != SIZE_MAX - 1) { // Reasonable limit for testing, avoid extreme values and zero
      Assert.isNotNull(ptr, "Memory allocation of size %zu should succeed", size);
      if (ptr) {
         Memory.dispose(ptr);
      }
   } else {
      // Very large allocations or zero should fail
      Assert.isNull(ptr, "Memory allocation of size %zu should fail", size);
   }
}

//  register test cases
__attribute__((constructor)) void init_fuzzing_tests(void) {
   testset("core_fuzzing_set", set_config, set_teardown);

   fuzz_testcase("Arena allocation fuzzing", test_arena_allocation_fuzz, FUZZ_SIZE_T);
   fuzz_testcase("FArray fuzzing", test_farray_fuzz, FUZZ_SIZE_T);
   fuzz_testcase("List fuzzing", test_list_fuzz, FUZZ_SIZE_T);
   fuzz_testcase("PArray fuzzing", test_parray_fuzz, FUZZ_SIZE_T);
   fuzz_testcase("SlotArray fuzzing", test_slotarray_fuzz, FUZZ_SIZE_T);
   fuzz_testcase("Memory allocation fuzzing", test_memory_allocation_fuzz, FUZZ_SIZE_T);
}