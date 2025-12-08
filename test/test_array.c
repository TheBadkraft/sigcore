/*
 *  Test File: test_array.c
 *  Description: Test cases for SigmaCore array interfaces
 */

#include "sigcore/array.h"
#include <sigtest/sigtest.h>
#include <stdio.h>
#include <string.h>

static void set_config(FILE **log_stream) {
   *log_stream = fopen("logs/test_array.log", "w");
}

//  basic initialization, disposal, and properties
static void test_array_new(void) {
   int initial_capacity = 10;
   array arr = Array.new(initial_capacity);
   Assert.isNotNull(arr, "Array creation failed");

   Array.dispose(arr);
}
static void test_array_init_from_null(void) {
   //  initialize &arr with capacity
   array arr = NULL;
   Array.init(&arr, 10);
   Assert.isNotNull(arr, "Array initialization failed");

   Array.dispose(arr);
}
static void test_array_init_existing(void) {
   //  create array first
   array arr = Array.new(5);
   Assert.isNotNull(arr, "Array creation failed");

   //  re-initialize with new capacity
   Array.init(&arr, 15);
   Assert.isNotNull(arr, "Array re-initialization failed");

   Array.dispose(arr);
}
static void test_array_get_capacity(void) {
   int exp_capacity = 20;
   array arr = Array.new(exp_capacity);
   Assert.isNotNull(arr, "Array creation failed");
   int act_capacity = Array.capacity(arr);
   Assert.areEqual(&exp_capacity, &act_capacity, INT, "Array capacity mismatch");
   Array.dispose(arr);
}
static void test_array_clear(void) {
   int initial_capacity = 10;
   array arr = Array.new(initial_capacity);
   Assert.isNotNull(arr, "Array creation failed");

   // Spoof the array to inject test values
   struct spoofed_array {
      addr *bucket;
      addr end;
   } *spoofed = (struct spoofed_array *)arr;

   // Set some non-zero values
   const char *test_values[] = {"foo", "bar", "baz", "goo", "doo"};
   for (int i = 0; i < 5; i++) {
      spoofed->bucket[i] = (addr)test_values[i];
   }

   // Verify values are set
   for (int i = 0; i < 5; i++) {
      const char *stored_value = (const char *)(spoofed->bucket[i]);
      Assert.isTrue(stored_value == test_values[i], "Pre-clear value mismatch at index %d", i);
   }

   // Now clear the array
   Array.clear(arr);

   // Verify all values are cleared to NULL (ADDR_EMPTY is 0)
   for (int i = 0; i < initial_capacity; i++) {
      const char *cleared_value = (const char *)(spoofed->bucket[i]);
      Assert.isNull((object)cleared_value, "Post-clear value not NULL at index %d", i);
   }

   Array.dispose(arr);
}

//  array set value at index
static void test_array_set_value(void) {
   int initial_capacity = 10;
   array arr = Array.new(initial_capacity);
   // create sample int array
   int values[] = {1, 2, 3, 4, 5};
   // set values in array
   for (int i = 0; i < 5; i++) {
      Array.set(arr, i, (addr)values[i]);
   }
   // we can spoof the array here with an anonymous struct
   struct spoofed_array {
      addr *bucket;
      addr end;
   } *spoofed = (struct spoofed_array *)arr;
   // now verify values in spoofed array
   for (int i = 0; i < 5; i++) {
      int stored_value = (int)(spoofed->bucket[i]);
      writelnf("\tStored value at index %d: exp: %d  act: %d", i, values[i], stored_value);
      Assert.areEqual(&values[i], &stored_value, INT, "Array set value mismatch at index %d", i);
   }

   Array.dispose(arr);
}
static void test_array_get_value(void) {
   int initial_capacity = 10;
   array arr = Array.new(initial_capacity);
   // create sample int array
   int values[] = {10, 20, 30, 40, 50};
   // set values in array
   for (int i = 0; i < 5; i++) {
      Array.set(arr, i, (addr)values[i]);
   }

   for (int i = 0; i < 5; i++) {
      addr element = ADDR_EMPTY;
      Assert.isTrue(Array.get(arr, i, &element) == 0, "Array get failed at index %d", i);
      writelnf("\tRetrieved value at index %d: exp: %d  act: %d", i, values[i], element);
      Assert.areEqual(&values[i], &element, INT, "Array get value mismatch at index %d", i);
   }

   Array.dispose(arr);
}
static void test_array_remove_at(void) {
   int initial_capacity = 5;
   array arr = Array.new(initial_capacity);
   // Set initial values: 10, 20, 30, 40, 50
   int initial_values[] = {10, 20, 30, 40, 50};
   for (int i = 0; i < 5; i++) {
      Array.set(arr, i, (addr)initial_values[i]);
   }

   // Remove at index 2 (removes 30)
   Array.remove(arr, 2);

   // Expected after removal: 10, 20, 40, 50, 0
   int expected_values[] = {10, 20, 40, 50, 0};
   struct spoofed_array {
      addr *bucket;
      addr end;
   } *spoofed = (struct spoofed_array *)arr;

   for (int i = 0; i < initial_capacity; i++) {
      int actual_value = (int)(spoofed->bucket[i]);
      Assert.areEqual(&expected_values[i], &actual_value, INT, "Array remove_at mismatch at index %d", i);
   }

   Array.dispose(arr);
}

//  register test cases
__attribute__((constructor)) void init_array_tests(void) {
   testset("core_array_set", set_config, NULL);

   testcase("array_creation", test_array_new);
   testcase("array_init_from_null", test_array_init_from_null);
   testcase("array_init_from_exisiting", test_array_init_existing);
   testcase("array_get_capacity", test_array_get_capacity);
   testcase("array_clear", test_array_clear);

   testcase("array_set_value", test_array_set_value);
   testcase("array_get_value", test_array_get_value);
   testcase("array_remove_at", test_array_remove_at);
}