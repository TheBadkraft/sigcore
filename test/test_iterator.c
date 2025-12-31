/*
 * Test file for iterator utilities
 */
#include "sigcore/collections.h"
#include "sigcore/memory.h"
#include <sigtest/sigtest.h>
#include <string.h>

static void set_config(FILE **log_stream) {
   *log_stream = fopen("logs/test_iterator.log", "w");
   // Set memory hooks to use sigtest's wrapped functions for tracking
   Memory.set_alloc_hooks(__wrap_malloc, __wrap_free, NULL, NULL);
}

static void set_teardown(void) {
   Memory.reset_alloc_hooks();
}

// Test iterator over a collection
void test_iterator_basic(void) {
   // Create an array of integers
   int data[] = {10, 20, 30, 40, 50};
   size_t num_elements = sizeof(data) / sizeof(data[0]);

   // Create a collection view of the array
   collection coll = Collections.create_view(data, data + num_elements, sizeof(int), num_elements, false);
   Assert.isNotNull(coll, "Collection creation failed");

   // Create an iterator
   iterator it = Collections.create_iterator(coll);
   Assert.isNotNull(it, "Iterator creation failed");

   // Test iteration
   int expected[] = {10, 20, 30, 40, 50};
   size_t index = 0;

   while (Iterator.next(it)) {
      object item = Iterator.current(it);
      int *value = (int *)item;
      Assert.areEqual(&expected[index], value, INT, "Iterator value mismatch at index %zu", index);
      index++;
   }

   Assert.areEqual(&num_elements, &index, LONG, "Iterator did not iterate over all elements");

   // Test reset
   Iterator.reset(it);
   bool has_next = Iterator.next(it);
   Assert.isTrue(has_next, "Iterator reset failed");
   object item = Iterator.current(it);
   Assert.isNotNull(item, "Iterator current after next failed");
   int *first_value = (int *)item;
   Assert.areEqual(&expected[0], first_value, INT, "Iterator reset did not return first element");

   // Test current without advancing
   Iterator.reset(it);
   object curr = Iterator.current(it);
   Assert.isNull(curr, "Iterator current before next should be NULL");

   // Next should advance and set current
   has_next = Iterator.next(it);
   Assert.isTrue(has_next, "Iterator next failed");
   curr = Iterator.current(it);
   Assert.isNotNull(curr, "Iterator current after next failed");
   int *curr_value = (int *)curr;
   Assert.areEqual(&expected[0], curr_value, INT, "Iterator current mismatch");

   // Dispose
   Iterator.dispose(it);
   Collections.dispose(coll);
}

// Register tests
__attribute__((constructor)) void init_iterator_tests(void) {
   testset("core_iterator_set", set_config, set_teardown);

   testcase("Iterator basic", test_iterator_basic);
}