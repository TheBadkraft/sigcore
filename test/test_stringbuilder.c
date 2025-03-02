// test_stringbuilder.c
#include "sigtest.h"
#include "sigcore.h"
#include <string.h>

    // Assert.isTrue(condition, "fail message");
    // Assert.isFalse(condition, "fail message");
    // Assert.areEqual(obj1, obj2, INT, "fail message");
    // Assert.areEqual(obj1, obj2, PTR, "fail message");
    // Assert.areEqual(obj1, obj2, STRING, "fail message");

/* create a new string builder */
void new_stringbuilder(void) {
	int expLength = 0;
	int expCapacity = 16;
	string_builder sb = StringBuilder.new(expCapacity);
	Assert.isTrue(sb != NULL, "StringBuilder.new failed to allocate");
	
	int actLength = StringBuilder.length(sb);
	int actCapacity = StringBuilder.capacity(sb);
	Assert.areEqual(&expLength, &actLength, INT, "Initial length should be 0");
	Assert.areEqual(&expCapacity, &actCapacity, INT, "Initial capacity should be 16");
	
	StringBuilder.free(sb);
}
/* stringbuilder to string */
void sb_to_string(void) {
	string expOutput = "Hello, World";
	
	string_builder sb = StringBuilder.new(16);
	StringBuilder.append(sb, expOutput);
	
	string actOutput = StringBuilder.toString(sb);
	Assert.isTrue(actOutput != NULL, "output is NULL");
	Assert.isTrue(strcmp(expOutput, actOutput) == 0, "strings are not identical");
	
	Mem.free(actOutput);
	StringBuilder.free(sb);
}
/* append plain string */
void append_sb(void) {
	int expLength = 0;
	int expCapacity = 16;
	string_builder sb = StringBuilder.new(expCapacity);
	
	string hello = "Hello";
	expLength = strlen(hello);
	StringBuilder.append(sb, hello);
	
	int actLength = StringBuilder.length(sb);
	printf("\n length=%d", actLength);
	Assert.areEqual(&expLength, &actLength, INT, "Length should be 5");
	
	StringBuilder.free(sb);
}
/* append formatted string */
void appendf_sb(void) {
	int expLength = 0;
	int expCapacity = 16;
	string expResult = "ID: 42, Name: Alice";
	expLength = strlen(expResult);
	
	string_builder sb = StringBuilder.new(expCapacity);
	Assert.isTrue(sb != NULL, "sb new failed");
	
	StringBuilder.appendf(sb, "ID: %d, Name: %s", 42, "Alice");
	int actLength = StringBuilder.length(sb);
	Assert.areEqual(&expLength, &actLength, INT, "length mismatch");
	
	string actResult = StringBuilder.toString(sb);
	Assert.isTrue(strcmp(actResult, expResult) == 0, "string mismatch");
	
	Mem.free(actResult);
	StringBuilder.free(sb);
}

// Register test cases
__attribute__((constructor)) void init_sigtest_tests(void) {
	register_test("new_stringbuilder", new_stringbuilder);
	register_test("sb_to_string", sb_to_string);
	register_test("append_sb", append_sb);
	register_test("appendf_sb", appendf_sb);
}
