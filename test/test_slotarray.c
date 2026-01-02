/*
 *  Test File: test_list.c
 *  Description: Test cases for SigmaCore array interfaces
 */

#include "sigcore/farray.h"
#include "sigcore/memory.h"
#include "sigcore/parray.h"
#include "sigcore/slotarray.h"
#include <sigtest/sigtest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//  configure test set
static void set_config(FILE **log_stream) {
   *log_stream = fopen("logs/test_slotarray.log", "w");
   // Set memory hooks to use sigtest's wrapped functions for tracking
   // Memory.set_alloc_hooks(__wrap_malloc, __wrap_free, NULL, NULL);
}

static void set_teardown(void) {
   // Memory.reset_alloc_hooks();
}

// basic initialization, disposal, and properties
static void test_slotarray_new(void) {
   int initial_capacity = 10;
   slotarray sa = SlotArray.new(initial_capacity);
   Assert.isNotNull(sa, "SlotArray creation ERRed");
   SlotArray.dispose(sa);
}
static void test_slotarray_dispose(void) {
   int initial_capacity = 10;
   slotarray sa = SlotArray.new(initial_capacity);
   Assert.isNotNull(sa, "SlotArray creation ERRed");

   // spoof the slotarray to access underlying buffer
   // struct sc_slotarray {
   //    struct {
   //       void *buffer;
   //       void *end;
   //    } array;
   //    usize stride;
   // } *spoofed = (struct sc_slotarray *)sa;
   // object allocated_buffer = spoofed->array.buffer;

   SlotArray.dispose(sa);
   // after disposal, the allocated buffer should be freed
   // Assert.isFalse(Memory.is_tracking(allocated_buffer), "SlotArray disposal ERRed to free underlying buffer");
   // Assert.isFalse(Memory.is_tracking(sa), "SlotArray disposal ERRed to free slotarray structure");
}

// data manipulation tests
static void test_slotarray_add_value(void) {
   slotarray sa = SlotArray.new(5);
   // create a data record
   int *p1 = Memory.alloc(sizeof(int), false);
   *p1 = 42;
   // add to slotarray
   int handle = SlotArray.add(sa, p1);
   Assert.isTrue(handle >= 0, "SlotArray add ERRed");

   // retrieve the value back and check it matches
   object retrieved = NULL;
   int result = SlotArray.get_at(sa, handle, &retrieved);
   Assert.areEqual(&(int){0}, &result, INT, "SlotArray get_at ERRed");
   Assert.areEqual(p1, retrieved, PTR, "SlotArray add/get pointer mismatch");

   Memory.dispose(p1);
   SlotArray.dispose(sa);
}
static void test_slotarray_get_value(void) {
   slotarray sa = SlotArray.new(5);
   int *expValue = Memory.alloc(sizeof(int), false);
   *expValue = 99;
   int handle = SlotArray.add(sa, expValue);
   Assert.isTrue(handle >= 0, "SlotArray add ERRed");

   // retrieve value at handle
   object retrieved = NULL;
   int result = SlotArray.get_at(sa, handle, &retrieved);
   Assert.areEqual(&(int){0}, &result, INT, "SlotArray get_at ERRed at handle %d", handle);
   Assert.isNotNull(retrieved, "SlotArray get_at returned NULL at handle %d", handle);
   //  the pointers should match
   Assert.areEqual(expValue, retrieved, PTR, "SlotArray get_at pointer mismatch");

   // for sanity, check retrieved data
   int *actValue = (int *)retrieved;
   Assert.areEqual(&expValue[0], &actValue[0], INT, "SlotArray get_at value mismatch");

   Memory.dispose(expValue);
   SlotArray.dispose(sa);
}
static void test_slotarray_remove_at(void) {
   slotarray sa = SlotArray.new(5);
   int *expValue = Memory.alloc(sizeof(int), false);
   *expValue = 123;
   int handle = SlotArray.add(sa, expValue);
   Assert.isTrue(handle >= 0, "SlotArray add ERRed");

   // remove at handle
   int result = SlotArray.remove_at(sa, handle);
   Assert.areEqual(&(int){0}, &result, INT, "SlotArray remove_at ERRed at handle %d", handle);

   // try to get value at handle, should ERR
   object retrieved = NULL;
   result = SlotArray.get_at(sa, handle, &retrieved);
   Assert.areEqual(&(int){-1}, &result, INT, "SlotArray get_at should ERR after remove at handle %d", handle);

   Memory.dispose(expValue);
   SlotArray.dispose(sa);
}

// test that slotarray does not grow beyond initial capacity
static void test_slotarray_no_growth(void) {
   slotarray sa = SlotArray.new(3); // Small initial capacity

   // Add items up to capacity
   int *p1 = Memory.alloc(sizeof(int), false);
   *p1 = 1;
   int *p2 = Memory.alloc(sizeof(int), false);
   *p2 = 2;
   int *p3 = Memory.alloc(sizeof(int), false);
   *p3 = 3;
   int *p4 = Memory.alloc(sizeof(int), false);
   *p4 = 4;

   int h1 = SlotArray.add(sa, p1);
   int h2 = SlotArray.add(sa, p2);
   int h3 = SlotArray.add(sa, p3);
   int h4 = SlotArray.add(sa, p4); // This should fail

   Assert.isTrue(h1 >= 0, "First add ERRed");
   Assert.isTrue(h2 >= 0, "Second add ERRed");
   Assert.isTrue(h3 >= 0, "Third add ERRed");
   Assert.areEqual(&(int){-1}, &h4, INT, "Fourth add should ERR (no growth)");

   // Verify capacity remains 3
   usize capacity = SlotArray.capacity(sa);
   Assert.areEqual(&(int){3}, &(int){capacity}, INT, "Capacity should remain 3");

   // Verify first three values are accessible
   object retrieved;
   Assert.areEqual(&(int){0}, &(int){SlotArray.get_at(sa, h1, &retrieved)}, INT, "Get h1 ERRed");
   Assert.areEqual(p1, retrieved, PTR, "h1 value mismatch");

   Assert.areEqual(&(int){0}, &(int){SlotArray.get_at(sa, h2, &retrieved)}, INT, "Get h2 ERRed");
   Assert.areEqual(p2, retrieved, PTR, "h2 value mismatch");

   Assert.areEqual(&(int){0}, &(int){SlotArray.get_at(sa, h3, &retrieved)}, INT, "Get h3 ERRed");
   Assert.areEqual(p3, retrieved, PTR, "h3 value mismatch");

   Memory.dispose(p1);
   Memory.dispose(p2);
   Memory.dispose(p3);
   Memory.dispose(p4);
   SlotArray.dispose(sa);
}
static void test_slotarray_is_empty_slot(void) {
   slotarray sa = SlotArray.new(5);

   // Initially all slots should be empty
   Assert.isTrue(SlotArray.is_empty_slot(sa, 0), "Slot 0 should be empty initially");
   Assert.isTrue(SlotArray.is_empty_slot(sa, 2), "Slot 2 should be empty initially");

   // Add something
   int *p = Memory.alloc(sizeof(int), false);
   *p = 42;
   int handle = SlotArray.add(sa, p);
   Assert.isTrue(handle >= 0, "Add ERRed");

   // The added slot should not be empty
   Assert.isFalse(SlotArray.is_empty_slot(sa, handle), "Added slot should not be empty");

   // Other slots should still be empty
   for (usize i = 0; i < SlotArray.capacity(sa); i++) {
      if (i != (usize)handle) {
         Assert.isTrue(SlotArray.is_empty_slot(sa, i), "Slot %zu should be empty", i);
      }
   }

   // Remove it
   SlotArray.remove_at(sa, handle);

   // Now it should be empty again
   Assert.isTrue(SlotArray.is_empty_slot(sa, handle), "Removed slot should be empty again");

   Memory.dispose(p);
   SlotArray.dispose(sa);
}

// negative & edge test cases
static void test_slotarray_capacity(void) {
   slotarray sa = SlotArray.new(10);
   usize capacity = SlotArray.capacity(sa);
   Assert.areEqual(&(int){10}, &(int){capacity}, INT, "Initial capacity should be 10");

   // Add some items
   int *p1 = Memory.alloc(sizeof(int), false);
   *p1 = 1;
   int *p2 = Memory.alloc(sizeof(int), false);
   *p2 = 2;
   SlotArray.add(sa, p1);
   SlotArray.add(sa, p2);

   // Capacity should still be 10 (no growth yet)
   capacity = SlotArray.capacity(sa);
   Assert.areEqual(&(int){10}, &(int){capacity}, INT, "Capacity should remain 10 after adding 2 items");

   Memory.dispose(p1);
   Memory.dispose(p2);
   SlotArray.dispose(sa);
}
static void test_slotarray_clear(void) {
   slotarray sa = SlotArray.new(5);

   // Add some items
   int *p1 = Memory.alloc(sizeof(int), false);
   *p1 = 1;
   int *p2 = Memory.alloc(sizeof(int), false);
   *p2 = 2;
   int h1 = SlotArray.add(sa, p1);
   int h2 = SlotArray.add(sa, p2);

   // Verify they're there
   object retrieved;
   Assert.areEqual(&(int){0}, &(int){SlotArray.get_at(sa, h1, &retrieved)}, INT, "Get h1 ERRed before clear");
   Assert.areEqual(&(int){0}, &(int){SlotArray.get_at(sa, h2, &retrieved)}, INT, "Get h2 ERRed before clear");

   // Clear
   SlotArray.clear(sa);

   // Now they should be gone
   Assert.areEqual(&(int){-1}, &(int){SlotArray.get_at(sa, h1, &retrieved)}, INT, "Get h1 should ERR after clear");
   Assert.areEqual(&(int){-1}, &(int){SlotArray.get_at(sa, h2, &retrieved)}, INT, "Get h2 should ERR after clear");

   // All slots should be empty
   for (usize i = 0; i < SlotArray.capacity(sa); i++) {
      Assert.isTrue(SlotArray.is_empty_slot(sa, i), "Slot %zu should be empty after clear", i);
   }

   Memory.dispose(p1);
   Memory.dispose(p2);
   SlotArray.dispose(sa);
}

// stress test for slotarray
static void test_slotarray_stress(void) {
   const usize INITIAL_CAPACITY = 10;

   slotarray sa = SlotArray.new(INITIAL_CAPACITY);
   int handles[50];          // Store handles
   int *values[50];          // Store allocated values
   bool valid[50] = {false}; // Track which values are still allocated

   // Phase 1: Fill to capacity
   for (usize i = 0; i < 10; i++) {
      values[i] = Memory.alloc(sizeof(int), false);
      *values[i] = (int)i;
      valid[i] = true;
      handles[i] = SlotArray.add(sa, values[i]);
      Assert.isTrue(handles[i] >= 0, "Add %zu ERRed", i);
   }

   // Phase 2: Remove every other item (create holes)
   for (usize i = 0; i < 10; i += 2) {
      int result = SlotArray.remove_at(sa, (usize)handles[i]);
      Assert.areEqual(&(int){0}, &result, INT, "Remove %zu ERRed", i);
      Memory.dispose(values[i]); // Free the actual memory
      valid[i] = false;          // Mark as freed
   }

   // Phase 3: Add new items (should reuse freed slots)
   for (usize i = 10; i < 15; i++) {
      values[i] = Memory.alloc(sizeof(int), false);
      *values[i] = (int)i;
      valid[i] = true;
      handles[i] = SlotArray.add(sa, values[i]);
      Assert.isTrue(handles[i] >= 0, "Reuse add %zu ERRed", i);
   }

   // Phase 4: Verify all remaining items are accessible
   for (usize i = 1; i < 15; i += 2) { // Check odd indices (not removed)
      if (valid[i]) {
         object retrieved;
         int result = SlotArray.get_at(sa, (usize)handles[i], &retrieved);
         Assert.areEqual(&(int){0}, &result, INT, "Get remaining item %zu ERRed", i);
         Assert.areEqual(values[i], retrieved, PTR, "Retrieved value %zu mismatch", i);
         Assert.areEqual(&(int){i}, (int *)retrieved, INT, "Retrieved value content %zu mismatch", i);
      }
   }

   // Phase 5: Remove all remaining items
   for (usize i = 1; i < 15; i += 2) {
      if (valid[i]) {
         int result = SlotArray.remove_at(sa, (usize)handles[i]);
         Assert.areEqual(&(int){0}, &result, INT, "Final remove %zu ERRed", i);
         Memory.dispose(values[i]);
         valid[i] = false;
      }
   }

   // Phase 6: Add a few more to verify slot reuse
   for (usize i = 15; i < 20; i++) {
      values[i] = Memory.alloc(sizeof(int), false);
      *values[i] = (int)(i + 100);
      valid[i] = true;
      handles[i] = SlotArray.add(sa, values[i]);
      Assert.isTrue(handles[i] >= 0, "Final reuse add %zu ERRed", i);

      object retrieved;
      Assert.areEqual(&(int){0}, &(int){SlotArray.get_at(sa, (usize)handles[i], &retrieved)}, INT, "Final get %zu ERRed", i);
      Assert.areEqual(values[i], retrieved, PTR, "Final retrieved value %zu mismatch", i);
   }

   // Cleanup all remaining valid allocations
   for (usize i = 0; i < 20; i++) {
      if (valid[i]) {
         Memory.dispose(values[i]);
      }
   }

   SlotArray.dispose(sa);
}

// test creating slotarray from parray
static void test_slotarray_from_pointer_array(void) {
   parray arr = PArray.new(5);
   int values[] = {10, 20, 30, 40, 50};
   for (int i = 0; i < 5; i++) {
      PArray.set(arr, i, (addr)values[i]);
   }

   slotarray sa = SlotArray.from_pointer_array(arr);
   Assert.isNotNull(sa, "SlotArray from_pointer_array ERRed");

   // check that values are copied
   object retrieved;
   for (int i = 0; i < 5; i++) {
      Assert.areEqual(&(int){OK}, &(int){SlotArray.get_at(sa, i, &retrieved)}, INT, "SlotArray get_at ERRed");
      Assert.areEqual((object)(addr)values[i], retrieved, PTR, "SlotArray value mismatch");
   }

   SlotArray.dispose(sa);
   PArray.dispose(arr);
}

// test creating slotarray from farray
static void test_slotarray_from_value_array(void) {
   usize element_size = sizeof(int);
   farray arr = FArray.new(5, element_size);
   int values[] = {10, 20, 30, 40, 50};
   for (int i = 0; i < 5; i++) {
      FArray.set(arr, i, element_size, &values[i]);
   }

   slotarray sa = SlotArray.from_value_array(arr, element_size);
   Assert.isNotNull(sa, "SlotArray from_value_array ERRed");

   // check that values are copied
   object retrieved;
   for (int i = 0; i < 5; i++) {
      Assert.areEqual(&(int){OK}, &(int){SlotArray.get_at(sa, i, &retrieved)}, INT, "SlotArray get_at ERRed");
      Assert.areEqual(&(int){values[i]}, (int *)retrieved, INT, "SlotArray value mismatch");
   }

   // free the allocated objects
   for (int i = 0; i < 5; i++) {
      if (SlotArray.get_at(sa, i, &retrieved) == OK) {
         free(retrieved);
      }
   }

   SlotArray.dispose(sa);
   FArray.dispose(arr);
}

//  register test cases
__attribute__((constructor)) void init_slotarray_tests(void) {
   testset("core_slotarray_set", set_config, set_teardown);

   testcase("slotarray_creation", test_slotarray_new);
   testcase("slotarray_dispose", test_slotarray_dispose);
   // No Size with SlotArray

   testcase("slotarray_set_add", test_slotarray_add_value);
   testcase("slotarray_try_get_value", test_slotarray_get_value);
   testcase("slotarray_remove_at", test_slotarray_remove_at);

   testcase("slotarray_no_growth", test_slotarray_no_growth);
   testcase("slotarray_is_valid_index", test_slotarray_is_empty_slot);

   testcase("slotarray_get_capacity", test_slotarray_capacity);
   testcase("slotarray_clear", test_slotarray_clear);
   testcase("slotarray_stress", test_slotarray_stress);
   testcase("slotarray_from_pointer_array", test_slotarray_from_pointer_array);
   testcase("slotarray_from_value_array", test_slotarray_from_value_array);
}