/*
 * Test file for string builder utilities
 */
#include "sigcore/memory.h"
#include "sigcore/strings.h"
#include <sigtest/sigtest.h>
#include <string.h>

static void set_config(FILE **log_stream) {
   *log_stream = fopen("logs/test_stringbuilder.log", "w");
}

// Test new stringbuilder
void test_new_stringbuilder(void) {
   size_t expLength = 0;
   size_t expCapacity = 16;
   string_builder sb = StringBuilder.new(expCapacity);
   Assert.isNotNull(sb, "StringBuilder.new failed to allocate");

   size_t actLength = StringBuilder.length(sb);
   size_t actCapacity = StringBuilder.capacity(sb);
   Assert.areEqual(&expLength, &actLength, LONG, "Initial length should be 0");
   Assert.areEqual(&expCapacity, &actCapacity, LONG, "Initial capacity should be 16");

   StringBuilder.dispose(sb);
} // Test clear stringbuilder
void test_sb_clear(void) {
   string_builder sb = StringBuilder.snew("Hello");
   size_t expLength = 0;

   StringBuilder.clear(sb);
   size_t actLength = StringBuilder.length(sb);

   Assert.areEqual(&expLength, &actLength, LONG, "sb length should be 0");

   StringBuilder.dispose(sb);
} // Test stringbuilder to string
void test_sb_tostring(void) {
   string expOutput = "Hello, World";

   string_builder sb = StringBuilder.new(16);
   StringBuilder.append(sb, expOutput);

   string actOutput = StringBuilder.toString(sb);
   Assert.isNotNull(actOutput, "output is NULL");
   Assert.areEqual(&(int){0}, &(int){strcmp(expOutput, actOutput)}, INT, "strings are not identical");

   String.dispose(actOutput);
   StringBuilder.dispose(sb);
} // Test append to empty sb
void test_append_empty_sb(void) {
   size_t expLength = 5; // "Hello"
   string_builder sb = StringBuilder.new(16);

   string hello = "Hello";
   StringBuilder.append(sb, hello);

   size_t actLength = StringBuilder.length(sb);
   Assert.areEqual(&expLength, &actLength, LONG, "Length should be 5");

   StringBuilder.dispose(sb);
} // Test append formatted string
void test_appendf_sb(void) {
   size_t expLength = 6; // "ID: 42"
   string expResult = "ID: 42";
   string_builder sb = StringBuilder.new(16);
   Assert.isNotNull(sb, "sb new failed");

   StringBuilder.appendf(sb, "ID: %d", 42);
   size_t actLength = StringBuilder.length(sb);
   Assert.areEqual(&expLength, &actLength, LONG, "length mismatch");

   string actResult = StringBuilder.toString(sb);
   Assert.areEqual(&(int){0}, &(int){strcmp(actResult, expResult)}, INT, "string mismatch");

   String.dispose(actResult);
   StringBuilder.dispose(sb);
} // Test new sb from string
void test_snew_sb(void) {
   string str = "A char* buffer";
   size_t expLength = strlen(str);

   string_builder sb = StringBuilder.snew(str);
   size_t actLength = StringBuilder.length(sb);

   Assert.areEqual(&expLength, &actLength, LONG, "length mismatch");

   StringBuilder.dispose(sb);
}

// Test append line
void test_appendl_sb(void) {
   string str = "A char* buffer";
   string appStr = "with appended line";

   string_builder sb = StringBuilder.snew(str);
   StringBuilder.appendl(sb, appStr);
   size_t actLength = StringBuilder.length(sb);

   string expOutput = "A char* bufferwith appended line\n";
   string actOutput = StringBuilder.toString(sb);

   Assert.areEqual(&(size_t){strlen(expOutput)}, &actLength, LONG, "length mismatch");
   Assert.areEqual(&(int){0}, &(int){strcmp(expOutput, actOutput)}, INT, "string mismatch");

   String.dispose(actOutput);
   StringBuilder.dispose(sb);
}

// Register tests
__attribute__((constructor)) void init_stringbuilder_tests(void) {
   testset("core_stringbuilder_set", set_config, NULL);

   testcase("New stringbuilder", test_new_stringbuilder);
   testcase("Clear stringbuilder", test_sb_clear);
   testcase("To string", test_sb_tostring);
   testcase("Append empty", test_append_empty_sb);
   testcase("Append formatted", test_appendf_sb);
   testcase("Snew from string", test_snew_sb);
   testcase("Append line", test_appendl_sb);
}