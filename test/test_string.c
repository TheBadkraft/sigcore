/*
 * Test file for string utilities
 */
#include "sigcore/memory.h"
#include "sigcore/strings.h"
#include <sigtest/sigtest.h>
#include <string.h>

void set_config(FILE **log_stream) {
   *log_stream = fopen("logs/test_string.log", "w");
   // Set memory hooks to use sigtest's wrapped functions for tracking
   Memory.set_alloc_hooks(__wrap_malloc, __wrap_free, NULL, NULL);
}

void set_teardown(void) {
   Memory.reset_alloc_hooks();
}

// Test string length calculation
void test_get_length(void) {
   string str = "Hello, World!";
   size_t expLength = strlen(str);
   size_t actLength = String.length(str);

   Assert.areEqual(&expLength, &actLength, LONG, "Length mismatch");

   // test NULL string
   expLength = 0;
   actLength = String.length(NULL);
   Assert.areEqual(&expLength, &actLength, LONG, "Length should be 0 for NULL");
}

// Test copy string
void test_copy_string(void) {
   string original = "Test string";
   string copy = String.copy(original);

   Assert.isNotNull(copy, "copy failed");
   Assert.areEqual(&(int){0}, &(int){strcmp(original, copy)}, INT, "copy should match original");
   Assert.isFalse(original == copy, "copy should not be the same pointer");

   // test NULL copy
   string nullCopy = String.copy(NULL);
   Assert.isNull(nullCopy, "copy of NULL should be NULL");

   String.dispose(copy);
}

// Test string concatenation
void test_concat_string(void) {
   string str1 = "Hello, ";
   string str2 = "World!";
   string expOutput = "Hello, World!";

   string result = String.concat(str1, str2);
   Assert.isNotNull(result, "concat result failed");
   Assert.areEqual(&(int){0}, &(int){strcmp(expOutput, result)}, INT, "concat string mismatch");

   // test NULL concat
   string nullResult1 = String.concat(NULL, str2);
   Assert.isNull(nullResult1, "concat with 1st NULL should be NULL");

   string nullResult2 = String.concat(str1, NULL);
   Assert.isNull(nullResult2, "concat with 2nd NULL should be NULL");

   String.dispose(result);
}

// Test string comparison
void test_compare_string(void) {
   string str1 = "Route: A";
   string str2 = "Route: A";
   string str3 = "Route: B";

   Assert.areEqual(&(int){0}, &(int){String.compare(str1, str2)}, INT, "string compare mismatch (str1 & str2)");
   Assert.isFalse(String.compare(str1, str3) == 0, "string compare mismatch (str1 & str3)");
   Assert.isFalse(String.compare(NULL, str1) == 0, "NULL string compare mismatch");
}

// Test string format
void test_format_string(void) {
   string exp = "ID: 42";
   string result = String.format("ID: %d", 42);

   Assert.isNotNull(result, "string format returned NULL");
   Assert.areEqual(&(int){0}, &(int){String.compare(result, exp)}, INT, "string format mismatch");

   String.dispose(result);
}

// Test string duplicate
void test_dupe_string(void) {
   string original = "Hello";
   string dupe = String.dupe(original);

   Assert.areEqual(&(int){0}, &(int){String.compare(original, dupe)}, INT, "string dupe mismatch");

   String.dispose(dupe);
}

// Test string to array
void test_to_array(void) {
   string original = "Test string";
   char *arr = String.to_array(original);

   Assert.isNotNull(arr, "to_array failed");
   Assert.areEqual(&(int){0}, &(int){strcmp(original, arr)}, INT, "to_array should match original");
   Assert.isFalse(original == arr, "to_array should not be the same pointer");

   // test NULL to_array
   char *nullArr = String.to_array(NULL);
   Assert.isNull(nullArr, "to_array of NULL should be NULL");

   Memory.dispose(arr); // to_array returns allocated memory
}

// Register tests
__attribute__((constructor)) void init_strings_tests(void) {
   testset("core_strings_set", set_config, set_teardown);

   testcase("String length", test_get_length);
   testcase("String copy", test_copy_string);
   testcase("String concat", test_concat_string);
   testcase("String compare", test_compare_string);
   testcase("String format", test_format_string);
   testcase("String dupe", test_dupe_string);
   testcase("String to array", test_to_array);
}