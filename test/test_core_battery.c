/*
 *  Test File: test_core_battery.c
 *  Description: Comprehensive test battery for SigmaCore core functions and memory management
 */

#include "sigcore/arena.h"
#include "sigcore/farray.h"
#include "sigcore/list.h"
#include "sigcore/memory.h"
#include "sigcore/parray.h"
#include <sigtest/sigtest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test data structures
typedef struct {
   int id;
   char name[32];
   int value;
} TestItem;

// Memory tests
static void test_memory_basic_alloc_free(void) {
   void *ptr = Memory.alloc(64, false);
   Assert.isNotNull(ptr, "Memory.alloc should succeed");
   Memory.dispose(ptr);
}

static void test_memory_zero_alloc(void) {
   void *ptr = Memory.alloc(64, true);
   Assert.isNotNull(ptr, "Memory.alloc with zero should succeed");
   // Check that memory is zeroed
   int *int_ptr = (int *)ptr;
   Assert.areEqual(&(int){0}, int_ptr, INT, "Zero-allocated memory should be zeroed");
   Memory.dispose(ptr);
}

static void test_memory_realloc(void) {
   void *ptr = Memory.alloc(32, false);
   Assert.isNotNull(ptr, "Initial alloc should succeed");

   void *new_ptr = Memory.realloc(ptr, 64);
   Assert.isNotNull(new_ptr, "Realloc should succeed");

   Memory.dispose(new_ptr);
}

// Array tests
static void test_farray_basic(void) {
   usize element_size = sizeof(int);
   farray arr = FArray.new(10, element_size);
   Assert.isNotNull(arr, "FArray creation should succeed");

   int value = 42;
   int result = FArray.set(arr, 0, element_size, &value);
   Assert.areEqual(&(int){0}, &result, INT, "FArray set should succeed");

   int retrieved = 0;
   result = FArray.get(arr, 0, element_size, &retrieved);
   Assert.areEqual(&(int){0}, &result, INT, "FArray get should succeed");
   Assert.areEqual(&value, &retrieved, INT, "FArray get/set should work");

   FArray.dispose(arr);
}

static void test_parray_basic(void) {
   parray arr = PArray.new(10);
   Assert.isNotNull(arr, "PArray creation should succeed");

   int *value = Memory.alloc(sizeof(int), false);
   *value = 42;

   int result = PArray.set(arr, 0, (addr)value);
   Assert.areEqual(&(int){0}, &result, INT, "PArray set should succeed");

   addr retrieved = 0;
   result = PArray.get(arr, 0, &retrieved);
   Assert.areEqual(&(int){0}, &result, INT, "PArray get should succeed");
   Assert.areEqual((object)value, (object)retrieved, PTR, "PArray get should return same pointer");

   PArray.dispose(arr);
   Memory.dispose(value);
}

// List tests
static void test_list_basic(void) {
   list lst = List.new(5, sizeof(addr));
   Assert.isNotNull(lst, "List creation should succeed");

   TestItem *item = Memory.alloc(sizeof(TestItem), false);
   item->id = 1;
   strcpy(item->name, "test");
   item->value = 100;

   List.append(lst, item);

   usize size = List.size(lst);
   Assert.areEqual(&(usize){1}, &size, LONG, "List size should be 1");

   TestItem *retrieved = NULL;
   int result = List.get(lst, 0, (object *)&retrieved);
   Assert.areEqual(&(int){0}, &result, INT, "List get should succeed");
   Assert.areEqual(&item->id, &retrieved->id, INT, "List get should work");

   List.dispose(lst);
   Memory.dispose(item);
}

// Arena tests
static void test_arena_basic(void) {
   arena a = Memory.Arena.create(4); // 4 pages
   Assert.isNotNull(a, "Arena creation should succeed");

   void *ptr1 = Arena.alloc(a, 64, false);
   void *ptr2 = Arena.alloc(a, 32, false);

   Assert.isNotNull(ptr1, "Arena alloc should succeed");
   Assert.isNotNull(ptr2, "Arena alloc should succeed");

   Memory.Arena.dispose(a);
}

// Configuration
static void set_config(FILE **log_stream) {
   *log_stream = fopen("logs/test_core_battery.log", "w");
}

static void set_teardown(void) {
   // Cleanup
}

// Test set registration
__attribute__((constructor)) void init_core_battery_tests(void) {
   testset("core_battery_set", set_config, set_teardown);

   // Memory tests
   testcase("memory_basic_alloc_free", test_memory_basic_alloc_free);
   testcase("memory_zero_alloc", test_memory_zero_alloc);
   testcase("memory_realloc", test_memory_realloc);

   // Array tests
   testcase("farray_basic", test_farray_basic);
   testcase("parray_basic", test_parray_basic);

   // List tests
   testcase("list_basic", test_list_basic);

   // Arena tests
   testcase("arena_basic", test_arena_basic);
}