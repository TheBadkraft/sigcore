/*
 *  Test File: test_collections.c
 *  Description: Test cases for SigmaCore collections interfaces
 */

#include "sigcore/collections.h"
#include "sigcore/memory.h"
#include "sigcore/parray.h"
#include "sigcore/types.h"
#include <sigtest/sigtest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//  configure test set
static void set_config(FILE **log_stream) {
   *log_stream = fopen("logs/test_collections.log", "w");
}

//  test compact function
static void test_collections_compact(void) {
   // create an array with capacity 5
   parray arr = PArray.new(5);
   Assert.isNotNull(arr, "Array creation failed");

   // allocate some dummy objects
   object obj1 = Memory.alloc(1); // dummy object 1
   object obj2 = Memory.alloc(1); // dummy object 2
   object obj3 = Memory.alloc(1); // dummy object 3

   // set entries: [obj1, NULL, obj2, NULL, obj3]
   PArray.set(arr, 0, (addr)obj1);
   PArray.set(arr, 1, ADDR_EMPTY);
   PArray.set(arr, 2, (addr)obj2);
   PArray.set(arr, 3, ADDR_EMPTY);
   PArray.set(arr, 4, (addr)obj3);

   // compact the array
   usize compacted_count = Collections.compact(arr);
   Assert.areEqual(&(int){3}, (int *)&compacted_count, INT, "Compact should return count of non-empty elements");

   // check that non-empty entries are compacted to front
   addr entry0, entry1, entry2, entry3, entry4;
   PArray.get(arr, 0, &entry0);
   PArray.get(arr, 1, &entry1);
   PArray.get(arr, 2, &entry2);
   PArray.get(arr, 3, &entry3);
   PArray.get(arr, 4, &entry4);

   Assert.areEqual((object)obj1, (object)entry0, PTR, "Entry 0 mismatch after compact");
   Assert.areEqual((object)obj2, (object)entry1, PTR, "Entry 1 mismatch after compact");
   Assert.areEqual((object)obj3, (object)entry2, PTR, "Entry 2 mismatch after compact");
   Assert.areEqual((object)ADDR_EMPTY, (object)entry3, PTR, "Entry 3 should be empty after compact");
   Assert.areEqual((object)ADDR_EMPTY, (object)entry4, PTR, "Entry 4 should be empty after compact");

   // capacity should remain the same
   int cap = PArray.capacity(arr);
   Assert.areEqual(&(int){5}, &cap, INT, "Capacity changed after condense");

   // free resources
   Memory.free(obj1);
   Memory.free(obj2);
   Memory.free(obj3);
   PArray.dispose(arr);
}

//  register test cases
__attribute__((constructor)) void init_collections_tests(void) {
   testset("core_collections_set", set_config, NULL);

   testcase("collections_compact", test_collections_compact);
}