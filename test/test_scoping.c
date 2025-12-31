/*
 *  Test File: test_scoping.c
 *  Description: Test cases for SigmaCore scope transfer functionality
 */

#include "internal/memory_internal.h"
#include "sigcore/arena.h"
#include "sigcore/memory.h"
#include "sigcore/scope.h"
#include <sigtest/sigtest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void set_config(FILE **log_stream) {
   *log_stream = fopen("logs/test_scoping.log", "w");
}
static void set_teardown(void) {
   Memory.teardown();
}

// Test handle validation
void test_scope_handle_validation(void) {
   // Valid handles
   sc_arena *test_arena = Memory.create_arena(1);
   Assert.isNotNull(test_arena, "Arena creation should succeed");
   Assert.isTrue(memcmp((const char *)test_arena, "ARN", 4) == 0, "Arena handle should be 'ARN\\0'");

   frame test_frame = Arena.begin_frame(test_arena);
   Assert.isNotNull(test_frame, "Frame creation should succeed");
   Assert.isTrue(memcmp((const char *)test_frame, "FRM", 4) == 0, "Frame handle should be 'FRM\\0'");

   Arena.end_frame(test_frame);
   Memory.dispose_arena(test_arena);
}

// Test invalid handle rejection
void test_scope_invalid_handle_rejection(void) {
   sc_arena *test_arena = Memory.create_arena(1);
   Assert.isNotNull(test_arena, "Arena creation should succeed");

   void *ptr = Memory.alloc(64, false);
   Assert.isNotNull(ptr, "Memory allocation should succeed");

   // Invalid source handle
   char invalid_handle[4] = {'I', 'N', 'V', '\0'};
   int result = Memory.Scope.move(invalid_handle, test_arena, ptr);
   Assert.areEqual(&(int){result}, &(int){ERR}, INT, "Scope_transfer should reject invalid source handle");

   // Invalid destination handle
   result = Memory.Scope.move(test_arena, invalid_handle, ptr);
   Assert.areEqual(&(int){result}, &(int){ERR}, INT, "Scope_transfer should reject invalid destination handle");

   Memory.dispose(ptr);
   Memory.dispose_arena(test_arena);
}

// Test transfer from Memory to Arena (should be rejected)
void test_scope_transfer_memory_to_arena(void) {
   sc_arena *test_arena = Memory.create_arena(1);
   Assert.isNotNull(test_arena, "Arena creation should succeed");

   void *ptr = Memory.alloc(64, false);
   Assert.isNotNull(ptr, "Memory allocation should succeed");
   Assert.isTrue(Memory.is_tracking(ptr), "Pointer should be tracked by Memory initially");

   int result = Memory.Scope.move((void *)&Memory, test_arena, ptr);
   Assert.areEqual(&(int){result}, &(int){ERR}, INT, "Memory to Arena transfer should fail (invalid scope type)");
   Assert.isTrue(Memory.is_tracking(ptr), "Pointer should still be tracked by Memory");

   // Clean up
   Memory.dispose(ptr);
   Memory.dispose_arena(test_arena);
}

// Test transfer from Arena to Memory
void test_scope_transfer_arena_to_memory(void) {
   sc_arena *test_arena = Memory.create_arena(1);
   Assert.isNotNull(test_arena, "Arena creation should succeed");

   void *ptr = Arena.alloc(test_arena, 64, false);
   Assert.isNotNull(ptr, "Arena allocation should succeed");

   int result = Memory.Scope.move(test_arena, (void *)&Memory, ptr);
   Assert.areEqual(&(int){result}, &(int){ERR}, INT, "Arena to Memory transfer should fail (invalid scope type)");

   Memory.dispose_arena(test_arena);
}

// Test transfer from Frame to Arena
void test_scope_transfer_frame_to_arena(void) {
   sc_arena *source_arena = Memory.create_arena(1);
   sc_arena *dest_arena = Memory.create_arena(1);
   Assert.isNotNull(source_arena, "Source arena creation should succeed");
   Assert.isNotNull(dest_arena, "Destination arena creation should succeed");

   frame test_frame = Arena.begin_frame(source_arena);
   Assert.isNotNull(test_frame, "Frame creation should succeed");

   void *ptr = Arena.alloc(source_arena, 64, false); // Allocate within frame scope
   Assert.isNotNull(ptr, "Arena allocation should succeed");

   int result = Memory.Scope.move(test_frame, dest_arena, ptr);
   Assert.areEqual(&(int){result}, &(int){OK}, INT, "Scope_transfer from Frame to Arena should succeed");

   Arena.end_frame(test_frame);
   Memory.dispose_arena(source_arena);
   Memory.dispose_arena(dest_arena);
}

// Test transfer from Arena to Frame
void test_scope_transfer_arena_to_frame(void) {
   sc_arena *test_arena = Memory.create_arena(1);
   Assert.isNotNull(test_arena, "Arena creation should succeed");

   frame test_frame = Arena.begin_frame(test_arena);
   Assert.isNotNull(test_frame, "Frame creation should succeed");

   void *ptr = Arena.alloc(test_arena, 64, false);
   Assert.isNotNull(ptr, "Arena allocation should succeed");

   int result = Memory.Scope.move(test_arena, test_frame, ptr);
   Assert.areEqual(&(int){result}, &(int){OK}, INT, "Scope_transfer from Arena to Frame should succeed");

   Arena.end_frame(test_frame); // This should dispose the transferred pointer
   Memory.dispose_arena(test_arena);
}

// Test transfer between different arenas
void test_scope_transfer_between_arenas(void) {
   sc_arena *source_arena = Memory.create_arena(1);
   sc_arena *dest_arena = Memory.create_arena(1);
   Assert.isNotNull(source_arena, "Source arena creation should succeed");
   Assert.isNotNull(dest_arena, "Destination arena creation should succeed");

   void *ptr = Arena.alloc(source_arena, 64, false);
   Assert.isNotNull(ptr, "Arena allocation should succeed");

   int result = Memory.Scope.move(source_arena, dest_arena, ptr);
   Assert.areEqual(&(int){result}, &(int){OK}, INT, "Scope_transfer between arenas should succeed");

   Memory.dispose_arena(source_arena);
   Memory.dispose_arena(dest_arena);
}

// Test transfer between frames
void test_scope_transfer_between_frames(void) {
   sc_arena *test_arena = Memory.create_arena(1);
   Assert.isNotNull(test_arena, "Arena creation should succeed");

   frame source_frame = Arena.begin_frame(test_arena);
   frame dest_frame = Arena.begin_frame(test_arena);
   Assert.isNotNull(source_frame, "Source frame creation should succeed");
   Assert.isNotNull(dest_frame, "Destination frame creation should succeed");

   void *ptr = Arena.alloc(test_arena, 64, false);
   Assert.isNotNull(ptr, "Arena allocation should succeed");

   int result = Memory.Scope.move(source_frame, dest_frame, ptr);
   Assert.areEqual(&(int){result}, &(int){OK}, INT, "Scope_transfer between frames should succeed");

   Arena.end_frame(source_frame);
   Arena.end_frame(dest_frame); // This should dispose the transferred pointer
   Memory.dispose_arena(test_arena);
}

// Test transfer of unowned object fails
void test_scope_transfer_unowned_object_fails(void) {
   sc_arena *test_arena = Memory.create_arena(1);
   Assert.isNotNull(test_arena, "Arena creation should succeed");

   void *ptr = malloc(64); // Allocate outside SigmaCore
   Assert.isNotNull(ptr, "External allocation should succeed");

   int result = Memory.Scope.move((void *)&Memory, test_arena, ptr);
   Assert.areEqual(&(int){result}, &(int){ERR}, INT, "Scope_transfer of unowned object should fail");

   free(ptr);
   Memory.dispose_arena(test_arena);
}

// Test transfer with NULL parameters fails
void test_scope_transfer_null_parameters_fail(void) {
   sc_arena *source_arena = Memory.create_arena(1);
   sc_arena *dest_arena = Memory.create_arena(1);
   Assert.isNotNull(source_arena, "Source arena creation should succeed");
   Assert.isNotNull(dest_arena, "Destination arena creation should succeed");

   void *ptr = Arena.alloc(source_arena, 64, false);
   Assert.isNotNull(ptr, "Arena allocation should succeed");

   // NULL source
   int result = Memory.Scope.move(NULL, dest_arena, ptr);
   Assert.areEqual(&(int){result}, &(int){ERR}, INT, "Scope_transfer with NULL source should fail");

   // NULL destination
   result = Memory.Scope.move(source_arena, NULL, ptr);
   Assert.areEqual(&(int){result}, &(int){ERR}, INT, "Scope_transfer with NULL destination should fail");

   // NULL object
   result = Memory.Scope.move(source_arena, dest_arena, NULL);
   Assert.areEqual(&(int){result}, &(int){ERR}, INT, "Scope_transfer with NULL object should fail");

   Memory.dispose_arena(source_arena);
   Memory.dispose_arena(dest_arena);
}

//  register test cases
__attribute__((constructor)) void init_scoping_tests(void) {
   testset("core_scoping_set", set_config, set_teardown);

   testcase("Handle validation works", test_scope_handle_validation);
   testcase("Invalid handles rejected", test_scope_invalid_handle_rejection);
   testcase("Memory to Arena transfer rejected", test_scope_transfer_memory_to_arena);
   testcase("Arena to Memory transfer rejected", test_scope_transfer_arena_to_memory);
   testcase("Frame to Arena transfer", test_scope_transfer_frame_to_arena);
   testcase("Arena to Frame transfer", test_scope_transfer_arena_to_frame);
   testcase("Transfer between arenas", test_scope_transfer_between_arenas);
   testcase("Transfer between frames", test_scope_transfer_between_frames);
   testcase("Unowned object transfer fails", test_scope_transfer_unowned_object_fails);
   testcase("NULL parameters fail", test_scope_transfer_null_parameters_fail);
}