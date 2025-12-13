/*
 * Test file for string utilities
 */
#include "sigcore/core.h"
#include <sigtest/sigtest.h>
#include <stdio.h>
#include <string.h>

// Test string length
void test_string_length(void) {
    Assert.isTrue(String.length(NULL) == 0, "Length of NULL string is 0");
    Assert.isTrue(String.length("") == 0, "Length of empty string is 0");
    Assert.isTrue(String.length("hello") == 5, "Length of 'hello' is 5");
}

// Test string copy
void test_string_copy(void) {
    string original = "test";
    string copy = String.copy(original);
    Assert.isNotNull(copy, "Copy should not be NULL");
    Assert.isTrue(strcmp(copy, original) == 0, "Copy should equal original");
    String.free(copy);
}

// Test string duplicate
void test_string_duplicate(void) {
    const char *original = "duplicate";
    string dup = String.dupe(original);
    Assert.isNotNull(dup, "Duplicate should not be NULL");
    Assert.isTrue(strcmp(dup, original) == 0, "Duplicate should equal original");
    String.free(dup);
}

// Test string concat
void test_string_concat(void) {
    string result = String.concat("hello", "world");
    Assert.isNotNull(result, "Concat result should not be NULL");
    Assert.isTrue(strcmp(result, "helloworld") == 0, "Concat should work");
    String.free(result);
}

// Test string compare
void test_string_compare(void) {
    Assert.isTrue(String.compare("a", "a") == 0, "Equal strings");
    Assert.isTrue(String.compare("a", "b") < 0, "a < b");
    Assert.isTrue(String.compare("b", "a") > 0, "b > a");
}

// Test string format
void test_string_format(void) {
    string result = String.format("test %d", 42);
    Assert.isNotNull(result, "Format result should not be NULL");
    Assert.isTrue(strcmp(result, "test 42") == 0, "Format should work");
    String.free(result);
}

// Test string builder
void test_string_builder(void) {
    string_builder sb = StringBuilder.new(10);
    Assert.isNotNull(sb, "StringBuilder should be created");
    
    StringBuilder.append(sb, "hello");
    StringBuilder.append(sb, " ");
    StringBuilder.append(sb, "world");
    
    string result = StringBuilder.toString(sb);
    Assert.isNotNull(result, "toString should work");
    Assert.isTrue(strcmp(result, "hello world") == 0, "Append should work");
    
    String.free(result);
    StringBuilder.free(sb);
}

// Register tests
__attribute__((constructor)) void init_strings_tests(void) {
    testset("core_strings_set", NULL, NULL);
    testcase("String length", test_string_length);
    testcase("String copy", test_string_copy);
    testcase("String duplicate", test_string_duplicate);
    testcase("String concat", test_string_concat);
    testcase("String compare", test_string_compare);
    testcase("String format", test_string_format);
    testcase("String builder", test_string_builder);
}