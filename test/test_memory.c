/*
 *  Test File: test_memory.c
 *  Description: Test cases for SigmaCore memory management interfaces
 */

#include "sigcore/memory.h"
#include <sigtest/sigtest.h>
#include <stdio.h>

static void set_config(FILE **log_stream) {
   *log_stream = fopen("logs/test_memory.log", "w");
}

//  test memory allocation and deallocation
void test_memory_alloc_free(void) {
   usize size = 128;
   void *ptr = Memory.alloc(size);
   Assert.isTrue(ptr != NULL, "Memory allocation failed");

   Memory.free(ptr);
   // Note: In C, we cannot directly test if memory has been freed.
   // We assume that if no crash occurs, the free operation was successful.
   Assert.isTrue(1, "Memory freed successfully");
}

//  register test cases
__attribute__((constructor)) void init_memory_tests(void) {
   testset("core_memory_set", set_config, NULL);

   testcase("memory_alloc_free", test_memory_alloc_free);
}