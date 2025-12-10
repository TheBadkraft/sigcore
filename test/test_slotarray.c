/*
 *  Test File: test_list.c
 *  Description: Test cases for SigmaCore array interfaces
 */

#include "sigcore/farray.h"
#include "sigcore/memory.h"
#include "sigcore/slotarray.h"
#include <sigtest/sigtest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//  configure test set
static void set_config(FILE **log_stream) {
   *log_stream = fopen("logs/test_slotarray.log", "w");
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
   struct sc_slotarray {
      struct {
         void *buffer;
         void *end;
      } array;
      usize stride;
   } *spoofed = (struct sc_slotarray *)sa;
   object allocated_buffer = spoofed->array.buffer;

   SlotArray.dispose(sa);
   // after disposal, the allocated buffer should be freed
   Assert.isFalse(Memory.has(allocated_buffer), "SlotArray disposal ERRed to free underlying buffer");
   Assert.isFalse(Memory.has(sa), "SlotArray disposal ERRed to free slotarray structure");
}

// data manipulation tests
static void test_slotarray_add_value(void) {
   slotarray sa = SlotArray.new(5);
   // create a data record
   int *p1 = Memory.alloc(sizeof(int));
   *p1 = 42;
   // add to slotarray
   int handle = SlotArray.add(sa, p1);
   Assert.isTrue(handle >= 0, "SlotArray add ERRed");

   // spoof the slotarray to access underlying buffer
   struct sc_slotarray {
      struct {
         void *buffer;
         void *end;
      } array;
      usize stride;
   } *spoofed = (struct sc_slotarray *)sa;
   addr *bucket = (addr *)spoofed->array.buffer;
   int *actValue = (int *)bucket[handle];
   // just check for value equality
   Assert.areEqual(p1, actValue, PTR, "SlotArray add pointer mismatch");

   Memory.free(p1);
   SlotArray.dispose(sa);
}
static void test_slotarray_get_value(void) {
   slotarray sa = SlotArray.new(5);
   int *expValue = Memory.alloc(sizeof(int));
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

   Memory.free(expValue);
   SlotArray.dispose(sa);
}
static void test_slotarray_remove_at(void) {
   slotarray sa = SlotArray.new(5);
   int *expValue = Memory.alloc(sizeof(int));
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

   Memory.free(expValue);
   SlotArray.dispose(sa);
}

// advanced/bulk data manipulation tests (???)
static void test_slotarray_growth(void) {
   slotarray sa = SlotArray.new(3); // Small initial capacity

   // Add items until we exceed capacity
   int *p1 = Memory.alloc(sizeof(int));
   *p1 = 1;
   int *p2 = Memory.alloc(sizeof(int));
   *p2 = 2;
   int *p3 = Memory.alloc(sizeof(int));
   *p3 = 3;
   int *p4 = Memory.alloc(sizeof(int));
   *p4 = 4;

   int h1 = SlotArray.add(sa, p1);
   int h2 = SlotArray.add(sa, p2);
   int h3 = SlotArray.add(sa, p3);
   int h4 = SlotArray.add(sa, p4); // This should trigger growth

   Assert.isTrue(h1 >= 0, "First add ERRed");
   Assert.isTrue(h2 >= 0, "Second add ERRed");
   Assert.isTrue(h3 >= 0, "Third add ERRed");
   Assert.isTrue(h4 >= 0, "Fourth add (growth) ERRed");

   // Verify capacity grew
   usize capacity = SlotArray.capacity(sa);
   Assert.isTrue(capacity >= 4, "Capacity should have grown to at least 4, got %zu", capacity);

   // Verify all values are accessible
   object retrieved;
   Assert.areEqual(&(int){0}, &(int){SlotArray.get_at(sa, h1, &retrieved)}, INT, "Get h1 ERRed");
   Assert.areEqual(p1, retrieved, PTR, "h1 value mismatch");

   Assert.areEqual(&(int){0}, &(int){SlotArray.get_at(sa, h2, &retrieved)}, INT, "Get h2 ERRed");
   Assert.areEqual(p2, retrieved, PTR, "h2 value mismatch");

   Assert.areEqual(&(int){0}, &(int){SlotArray.get_at(sa, h3, &retrieved)}, INT, "Get h3 ERRed");
   Assert.areEqual(p3, retrieved, PTR, "h3 value mismatch");

   Assert.areEqual(&(int){0}, &(int){SlotArray.get_at(sa, h4, &retrieved)}, INT, "Get h4 ERRed");
   Assert.areEqual(p4, retrieved, PTR, "h4 value mismatch");

   Memory.free(p1);
   Memory.free(p2);
   Memory.free(p3);
   Memory.free(p4);
   SlotArray.dispose(sa);
}
static void test_slotarray_is_empty_slot(void) {
   slotarray sa = SlotArray.new(5);

   // Initially all slots should be empty
   Assert.isTrue(SlotArray.is_empty_slot(sa, 0), "Slot 0 should be empty initially");
   Assert.isTrue(SlotArray.is_empty_slot(sa, 2), "Slot 2 should be empty initially");

   // Add something
   int *p = Memory.alloc(sizeof(int));
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

   Memory.free(p);
   SlotArray.dispose(sa);
}

// negative & edge test cases
static void test_slotarray_capacity(void) {
   slotarray sa = SlotArray.new(10);
   usize capacity = SlotArray.capacity(sa);
   Assert.areEqual(&(int){10}, &(int){capacity}, INT, "Initial capacity should be 10");

   // Add some items
   int *p1 = Memory.alloc(sizeof(int));
   *p1 = 1;
   int *p2 = Memory.alloc(sizeof(int));
   *p2 = 2;
   SlotArray.add(sa, p1);
   SlotArray.add(sa, p2);

   // Capacity should still be 10 (no growth yet)
   capacity = SlotArray.capacity(sa);
   Assert.areEqual(&(int){10}, &(int){capacity}, INT, "Capacity should remain 10 after adding 2 items");

   Memory.free(p1);
   Memory.free(p2);
   SlotArray.dispose(sa);
}
static void test_slotarray_clear(void) {
   slotarray sa = SlotArray.new(5);

   // Add some items
   int *p1 = Memory.alloc(sizeof(int));
   *p1 = 1;
   int *p2 = Memory.alloc(sizeof(int));
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

   Memory.free(p1);
   Memory.free(p2);
   SlotArray.dispose(sa);
}

// stress test for slotarray
static void test_slotarray_stress(void) {
   const usize INITIAL_CAPACITY = 8;

   slotarray sa = SlotArray.new(INITIAL_CAPACITY);
   int handles[50];          // Store handles
   int *values[50];          // Store allocated values
   bool valid[50] = {false}; // Track which values are still allocated

   // Phase 1: Fill to capacity and trigger growth
   for (usize i = 0; i < 20; i++) { // More than initial capacity
      values[i] = Memory.alloc(sizeof(int));
      *values[i] = (int)i;
      valid[i] = true;
      handles[i] = SlotArray.add(sa, values[i]);
      Assert.isTrue(handles[i] >= 0, "Add %zu ERRed", i);
   }

   // Verify capacity grew
   usize capacity = SlotArray.capacity(sa);
   Assert.isTrue(capacity >= 20, "Capacity should have grown, got %zu", capacity);

   // Phase 2: Remove every other item (create holes)
   for (usize i = 0; i < 20; i += 2) {
      int result = SlotArray.remove_at(sa, (usize)handles[i]);
      Assert.areEqual(&(int){0}, &result, INT, "Remove %zu ERRed", i);
      Memory.free(values[i]); // Free the actual memory
      valid[i] = false;       // Mark as freed
   }

   // Phase 3: Add new items (should reuse freed slots)
   for (usize i = 20; i < 40; i++) {
      values[i] = Memory.alloc(sizeof(int));
      *values[i] = (int)i;
      valid[i] = true;
      handles[i] = SlotArray.add(sa, values[i]);
      Assert.isTrue(handles[i] >= 0, "Reuse add %zu ERRed", i);
   }

   // Phase 4: Verify all remaining items are accessible
   for (usize i = 1; i < 40; i += 2) { // Check odd indices (not removed)
      if (valid[i]) {
         object retrieved;
         int result = SlotArray.get_at(sa, (usize)handles[i], &retrieved);
         Assert.areEqual(&(int){0}, &result, INT, "Get remaining item %zu ERRed", i);
         Assert.areEqual(values[i], retrieved, PTR, "Retrieved value %zu mismatch", i);
         Assert.areEqual(&(int){i}, (int *)retrieved, INT, "Retrieved value content %zu mismatch", i);
      }
   }

   // Phase 5: Remove all remaining items
   for (usize i = 1; i < 40; i += 2) {
      if (valid[i]) {
         int result = SlotArray.remove_at(sa, (usize)handles[i]);
         Assert.areEqual(&(int){0}, &result, INT, "Final remove %zu ERRed", i);
         Memory.free(values[i]);
         valid[i] = false;
      }
   }

   // Phase 6: Add a few more to verify slot reuse
   for (usize i = 40; i < 45; i++) {
      values[i] = Memory.alloc(sizeof(int));
      *values[i] = (int)(i + 100);
      valid[i] = true;
      handles[i] = SlotArray.add(sa, values[i]);
      Assert.isTrue(handles[i] >= 0, "Final reuse add %zu ERRed", i);

      object retrieved;
      Assert.areEqual(&(int){0}, &(int){SlotArray.get_at(sa, (usize)handles[i], &retrieved)}, INT, "Final get %zu ERRed", i);
      Assert.areEqual(values[i], retrieved, PTR, "Final retrieved value %zu mismatch", i);
   }

   // Cleanup all remaining valid allocations
   for (usize i = 0; i < 45; i++) {
      if (valid[i]) {
         Memory.free(values[i]);
      }
   }

   SlotArray.dispose(sa);
}

//  register test cases
__attribute__((constructor)) void init_slotarray_tests(void) {
   testset("core_slotarray_set", set_config, NULL);

   testcase("slotarray_creation", test_slotarray_new);
   testcase("slotarray_dispose", test_slotarray_dispose);
   // No Size with SlotArray

   testcase("slotarray_set_add", test_slotarray_add_value);
   testcase("slotarray_try_get_value", test_slotarray_get_value);
   testcase("slotarray_remove_at", test_slotarray_remove_at);

   testcase("slotarray_growth", test_slotarray_growth);
   testcase("slotarray_is_valid_index", test_slotarray_is_empty_slot);

   testcase("slotarray_get_capacity", test_slotarray_capacity);
   testcase("slotarray_clear", test_slotarray_clear);
   testcase("slotarray_stress", test_slotarray_stress);
}