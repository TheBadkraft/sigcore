// test_string.c
#include "strings.h"
#include "sigcore.h"
#include "sigtest.h"
#include <string.h>

    // Assert.isTrue(condition, "fail message");
    // Assert.isFalse(condition, "fail message");
    // Assert.areEqual(obj1, obj2, INT, "fail message");
    // Assert.areEqual(obj1, obj2, PTR, "fail message");
    // Assert.areEqual(obj1, obj2, STRING, "fail message");

/* test string length calculation */
void get_length(void) {
	string str = "Hello, World!";
	size_t expLength = strlen(str);
	size_t actLength = String.length(str);
	
	Assert.areEqual(&expLength, &actLength, INT, "Length mismatch");
	
	//	test NULL string
	expLength = (size_t)(0);
	actLength = String.length(NULL);
	Assert.areEqual(&expLength, &actLength, INT, "Length should be 0");
}
/* test copy string */
void copy_string(void) {
	string original = "Test string";
	string copy = String.copy(original);
	
	Assert.isTrue(copy != NULL, "copy failed");
	Assert.areEqual(original, copy, STRING, "copy should match original");
	Assert.isFalse(original == copy, "copy should not be the same pointer");
	
	//	test NULL copy 
	string nullCopy = String.copy(NULL);
	Assert.isTrue(nullCopy == NULL, "copy of NULL failed");
	
	String.free(copy);
}
/* test string concatenation */
void concat_string(void) {
	string str1 = "Hello, ";
	string str2 = "World!";
	string expOutput = "Hello, World!";
	
	string result = String.concat(str1, str2);
	Assert.isTrue(result != NULL, "concat result failed");
	Assert.areEqual(expOutput, result, STRING, "concat string mismatch");
	
	// test NULL concat
	string nullResult1 = String.concat(NULL, str2);
	Assert.isTrue(nullResult1 == NULL, "concat with 1st NULL should be NULL");
	
	string nullResult2 = String.concat(str1, NULL);
	Assert.isTrue(nullResult2 == NULL, "concat with 2nd NULL should be NULL");
	
	String.free(result);
}
/* test string comparison */
void compare_string(void) {
	string str1 = "Route: A";
	string str2 = "Route: A";
	string str3 = "Route: B";
	
	Assert.isTrue(String.compare(str1, str2) == 0, "string compare mismatch (str1 & str2)");
	Assert.isTrue(String.compare(str1, str3) != 0, "string compare mismatch (str1 & str3)");
	Assert.isTrue(String.compare(NULL, str1) != 0, "NULL string compare mismatch");
}
/* test sring format */
void format_string(void) {
	string exp = "ID: 42";
	string result = String.format("ID: %d", 42);
	
	Assert.isTrue(result != NULL, "string format returned NULL");
	Assert.isTrue(String.compare(result, exp) == 0, "string compare mismatch");
	
	String.free(result);
}
/* test string duplicate */
void dupe_string(void) {
	const char* original = "Hello";
	string dupe = String.dupe(original);
	
	Assert.isTrue(String.compare(original, dupe) == 0, "string compare mismatch");
	
	String.free(dupe);
}

// Register test cases
__attribute__((constructor)) void init_sigtest_tests(void) {
    register_test("get_length", get_length);
    register_test("copy_string", copy_string);
    register_test("concat_string", concat_string);
    register_test("compare_string", compare_string);
    register_test("format_string", format_string);
    register_test("dupe_string", dupe_string);
}
