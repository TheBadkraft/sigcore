/*
 *  Test File: test_farray.c
 *  Description: Test cases for SigmaCore farray interfaces
 *
 *  The farray is a flexible collection primitive that allows storing
 *  elements of arbitrary size directly, without the overhead of pointer
 *  indirection. This is useful for small types like integers or structs
 *  where memory efficiency is important.
 */

#include "sigcore/farray.h"
#include <sigtest/sigtest.h>
#include <stdio.h>
#include <string.h>

//  configure test set
static void set_config(FILE **log_stream) {
   *log_stream = fopen("logs/test_farray.log", "w");
}

//  basic initialization, disposal, and properties
static void test_farray_new(void) {
   int initial_capacity = 10;
   usize element_size = sizeof(int);
   farray arr = FArray.new(initial_capacity, element_size);
   Assert.isNotNull(arr, "FArray creation failed");

   FArray.dispose(arr);
}
static void test_farray_init_from_null(void) {
   //  initialize &arr with capacity
   farray arr = NULL;
   usize element_size = sizeof(int);
   FArray.init(&arr, 10, element_size);
   Assert.isNotNull(arr, "FArray initialization failed");

   FArray.dispose(arr);
}
static void test_farray_init_existing(void) {
   //  create farray first
   usize element_size = sizeof(int);
   farray arr = FArray.new(5, element_size);
   Assert.isNotNull(arr, "FArray creation failed");

   //  re-initialize with new capacity
   FArray.init(&arr, 15, element_size);
   Assert.isNotNull(arr, "FArray re-initialization failed");

   FArray.dispose(arr);
}
static void test_farray_get_capacity(void) {
   int exp_capacity = 20;
   usize element_size = sizeof(int);
   farray arr = FArray.new(exp_capacity, element_size);
   Assert.isNotNull(arr, "FArray creation failed");
   int act_capacity = FArray.capacity(arr, element_size);
   Assert.areEqual(&exp_capacity, &act_capacity, INT, "FArray capacity mismatch");
   FArray.dispose(arr);
}

//  data manipulation tests
static void test_farray_clear(void) {
   int initial_capacity = 10;
   usize element_size = sizeof(int);
   farray arr = FArray.new(initial_capacity, element_size);
   Assert.isNotNull(arr, "FArray creation failed");

   // Spoof the farray to inject test values
   struct spoofed_farray {
      void *bucket;
      void *end;
   } *spoofed = (struct spoofed_farray *)arr;

   // Set some non-zero values
   int test_values[] = {1, 2, 3, 4, 5};
   for (int i = 0; i < 5; i++) {
      memcpy((char *)spoofed->bucket + i * element_size, &test_values[i], element_size);
   }

   // Verify values are set
   for (int i = 0; i < 5; i++) {
      int stored_value;
      memcpy(&stored_value, (char *)spoofed->bucket + i * element_size, element_size);
      Assert.areEqual(&test_values[i], &stored_value, INT, "Pre-clear value mismatch at index %d", i);
   }

   // Now clear the farray
   FArray.clear(arr, element_size);

   // Verify all values are cleared to zero
   for (int i = 0; i < initial_capacity; i++) {
      int cleared_value;
      memcpy(&cleared_value, (char *)spoofed->bucket + i * element_size, element_size);
      Assert.areEqual(&(int){0}, &cleared_value, INT, "Post-clear value not zero at index %d", i);
   }

   FArray.dispose(arr);
}
static void test_farray_set_value(void) {
   int initial_capacity = 10;
   usize element_size = sizeof(int);
   farray arr = FArray.new(initial_capacity, element_size);
   // create sample int array
   int values[] = {1, 2, 3, 4, 5};
   // set values in farray
   for (int i = 0; i < 5; i++) {
      Assert.areEqual(&(int){0}, &(int){FArray.set(arr, i, element_size, &values[i])}, INT, "FArray set failed at index %d", i);
   }
   // we can spoof the farray here with an anonymous struct
   struct spoofed_farray {
      void *bucket;
      void *end;
   } *spoofed = (struct spoofed_farray *)arr;
   // now verify values in spoofed farray
   for (int i = 0; i < 5; i++) {
      int stored_value;
      memcpy(&stored_value, (char *)spoofed->bucket + i * element_size, element_size);
      writelnf("\tStored value at index %d: exp: %d  act: %d", i, values[i], stored_value);
      Assert.areEqual(&values[i], &stored_value, INT, "FArray set value mismatch at index %d", i);
   }

   FArray.dispose(arr);
}
static void test_farray_get_value(void) {
   int initial_capacity = 10;
   usize element_size = sizeof(int);
   farray arr = FArray.new(initial_capacity, element_size);
   // create sample int array
   int values[] = {10, 20, 30, 40, 50};
   // set values in farray
   for (int i = 0; i < 5; i++) {
      FArray.set(arr, i, element_size, &values[i]);
   }

   for (int i = 0; i < 5; i++) {
      int element = 0;
      Assert.isTrue(FArray.get(arr, i, element_size, &element) == 0, "FArray get failed at index %d", i);
      writelnf("\tRetrieved value at index %d: exp: %d  act: %d", i, values[i], element);
      Assert.areEqual(&values[i], &element, INT, "FArray get value mismatch at index %d", i);
   }

   FArray.dispose(arr);
}
static void test_farray_remove_at(void) {
   int initial_capacity = 5;
   usize element_size = sizeof(int);
   farray arr = FArray.new(initial_capacity, element_size);
   // Set initial values: 10, 20, 30, 40, 50
   int initial_values[] = {10, 20, 30, 40, 50};
   for (int i = 0; i < 5; i++) {
      FArray.set(arr, i, element_size, &initial_values[i]);
   }

   // Remove at index 2 (removes 30) -- farray should not shift left
   Assert.areEqual(&(int){0}, &(int){FArray.remove(arr, 2, element_size)}, INT, "FArray remove failed at index 2");

   // Expected after removal: 10, 20, 0, 40, 50
   int expected_values[] = {10, 20, 0, 40, 50};
   struct spoofed_farray {
      void *bucket;
      void *end;
   } *spoofed = (struct spoofed_farray *)arr;

   for (int i = 0; i < initial_capacity; i++) {
      int actual_value;
      memcpy(&actual_value, (char *)spoofed->bucket + i * element_size, element_size);
      Assert.areEqual(&expected_values[i], &actual_value, INT, "FArray remove_at mismatch at index %d", i);
   }

   FArray.dispose(arr);
}

//  negative test cases
static void test_farray_set_out_of_bounds(void) {
   usize element_size = sizeof(int);
   farray arr = FArray.new(5, element_size);
   int value = 999;
   int result = FArray.set(arr, 10, element_size, &value);
   Assert.areEqual(&result, &((int){-1}), INT, "set out of bounds should return -1");
   FArray.dispose(arr);
}
static void test_farray_get_out_of_bounds(void) {
   usize element_size = sizeof(int);
   farray arr = FArray.new(5, element_size);
   int value = 0;
   int result = FArray.get(arr, 10, element_size, &value);
   Assert.areEqual(&result, &((int){-1}), INT, "get out of bounds should return -1");
   FArray.dispose(arr);
}
static void test_farray_remove_out_of_bounds(void) {
   usize element_size = sizeof(int);
   farray arr = FArray.new(5, element_size);
   int result = FArray.remove(arr, 10, element_size);
   Assert.areEqual(&result, &((int){-1}), INT, "remove out of bounds should return -1");
   FArray.dispose(arr);
}

//  register test cases
__attribute__((constructor)) void init_farray_tests(void) {
   testset("core_farray_set", set_config, NULL);

   testcase("farray_creation", test_farray_new);
   testcase("farray_init_from_null", test_farray_init_from_null);
   testcase("farray_init_from_existing", test_farray_init_existing);
   testcase("farray_get_capacity", test_farray_get_capacity);
   testcase("farray_clear", test_farray_clear);

   testcase("farray_set_value", test_farray_set_value);
   testcase("farray_get_value", test_farray_get_value);
   testcase("farray_remove_at", test_farray_remove_at);

   testcase("farray_set_out_of_bounds", test_farray_set_out_of_bounds);
   testcase("farray_get_out_of_bounds", test_farray_get_out_of_bounds);
   testcase("farray_remove_out_of_bounds", test_farray_remove_out_of_bounds);
}