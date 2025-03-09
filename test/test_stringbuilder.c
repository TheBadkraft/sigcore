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
/* new sb from string (char* buffer) */
void snew_sb(void) {
	string str = "A char* buffer";
	int expLength = strlen(str);
	
	printf("\nbase string '%s' length=%d\n", str, expLength);
	
	string_builder sb = StringBuilder.snew(str);
	size_t actLength = StringBuilder.length(sb);
	
	Assert.isTrue(expLength == actLength, "length mismatch");
	printf("sb string   '%s' length=%ld  : ", StringBuilder.toString(sb), actLength);
	
	StringBuilder.free(sb);
}
/* append line */
void appendl_sb(void) {
	string str = "A char* buffer";
	string appStr = "with appended line";
	
	int expLength = snprintf(NULL, 0, "%s%s\n", str, appStr);
	printf("\noutput length=%d\n", expLength);
	
	string_builder sb = StringBuilder.snew(str);
	StringBuilder.appendl(sb, appStr);
	size_t actLength = StringBuilder.length(sb);
	
	string expOutput = "A char* bufferwith appended line\n";
	string actOutput = StringBuilder.toString(sb);
	
	Assert.isTrue(expLength == actLength, "length mismatch");
	Assert.isTrue(strcmp(expOutput, actOutput) == 0, "string mismatch");
	printf(" (output)\n'%s' length=%ld  : ", actOutput, actLength);
	
	Mem.free(actOutput);
	StringBuilder.free(sb);
}
/* append line + string */
void lappends_sb(void) {
	string str = "A char* buffer";
	string appStr = "with appended line";
	
	int expLength = snprintf(NULL, 0, "%s\n%s", str, appStr);
	printf("\noutput length=%d\n", expLength);
	
	string_builder sb = StringBuilder.snew(str);
	StringBuilder.lappends(sb, appStr);
	size_t actLength = StringBuilder.length(sb);
	
	string expOutput = "A char* buffer\nwith appended line";
	string actOutput = StringBuilder.toString(sb);
	
	Assert.isTrue(expLength == actLength, "length mismatch");
	Assert.isTrue(strcmp(expOutput, actOutput) == 0, "string mismatch");
	printf(" (output)\n'%s' length=%ld  : ", actOutput, actLength);
	
	Mem.free(actOutput);
	StringBuilder.free(sb);
}
/* append line + formatted_string */
void lappendf_sb(void) {
	string str = "A char* buffer";
	string appStr = "with appended line";
	string fmtStr = "(formatted)";
	
	int expLength = snprintf(NULL, 0, "%s\n%s %s", str, appStr, fmtStr);
	printf("\noutput length=%d\n", expLength);
	
	string_builder sb = StringBuilder.snew(str);
	StringBuilder.lappendf(sb, "%s %s", appStr, fmtStr);
	size_t actLength = StringBuilder.length(sb);
	
	string expOutput = "A char* buffer\nwith appended line (formatted)";
	string actOutput = StringBuilder.toString(sb);
	
	Assert.isTrue(expLength == actLength, "length mismatch");
	Assert.isTrue(strcmp(expOutput, actOutput) == 0, "string mismatch");
	printf(" (output)\n'%s' length=%ld  : ", actOutput, actLength);
	
	Mem.free(actOutput);
	StringBuilder.free(sb);
}

// Register test cases
__attribute__((constructor)) void init_sigtest_tests(void) {
	register_test("new_stringbuilder", new_stringbuilder);
	register_test("sb_to_string", sb_to_string);
	register_test("append_sb", append_sb);
	register_test("appendf_sb", appendf_sb);
	register_test("snew_sb", snew_sb);
	register_test("appendl_sb", appendl_sb);
	register_test("lappends_sb", lappends_sb);
	register_test("lappendf_sb", lappendf_sb);
}
