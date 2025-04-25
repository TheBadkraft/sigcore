// test_index.c
#include <sigtest.h>
#include "sigcore.h"

// Assert.isTrue(condition, "fail message");
// Assert.isFalse(condition, "fail message");
// Assert.areEqual(obj1, obj2, INT, "fail message");
// Assert.areEqual(obj1, obj2, PTR, "fail message");
// Assert.areEqual(obj1, obj2, STRING, "fail message");

static int filter_nonempty(object value)
{
	return value != 0;
}

void slot_array_tests(void)
{
	fprintf(stdout, "\n");
	fflush(stdout);

	fprintf(stdout, "slot_array\n");
}
/* Test initialize slot array */
void test_new_index(void)
{
	fprintf(stdout, "\n");
	fflush(stdout);

	int expCap = 5, expCount = 0;
	slot_array slarr = SlotArray.new(expCap);

	int actCap = SlotArray.capacity(slarr);
	int actCount = SlotArray.count(slarr);

	Assert.isTrue(slarr != NULL, "slot array allocation failed");
	Assert.areEqual(&expCap, &actCap, INT, "capacity mismatch");
	Assert.areEqual(&expCount, &actCount, INT, "count mismatch");

	SlotArray.free(slarr);
}
/* Test add an item */
void test_add_item(void)
{
	fprintf(stdout, "\n");
	fflush(stdout);

	int cap = 5, expCount = 1;
	slot_array slarr = SlotArray.new(cap);
	object item = (object)0x12345678;

	SlotArray.add(slarr, item);

	int actCount = SlotArray.count(slarr);

	Assert.areEqual(&expCount, &actCount, INT, "count mismatch");

	SlotArray.free(slarr);
}
/* Test get item - pass */
void test_getItem_pass(void)
{
	fprintf(stdout, "\n");
	fflush(stdout);

	int cap = 5;
	int expValue = 1;
	int expGet = 1;

	slot_array slarr = SlotArray.new(cap);
	SlotArray.add(slarr, &expValue);

	object actValue;
	int actGet = SlotArray.tryGetAt(slarr, 0, &actValue);

	Assert.areEqual(&expGet, &actGet, INT, "get mismatch - expected 1");
	Assert.isTrue(actValue == &expValue, "item pointer mismatch");
	Assert.isTrue(expValue == *(int *)actValue, "item value mismatch");

	SlotArray.free(slarr);
}
/* Test get item - fail */
void test_getItem_fail(void)
{
	fprintf(stdout, "\n");
	fflush(stdout);

	int cap = 5;
	int expGet = 0;

	slot_array slarr = SlotArray.new(cap);
	// don't add item, index will be empty

	object actValue;
	int actGet = SlotArray.tryGetAt(slarr, 1, &actValue);

	Assert.areEqual(&expGet, &actGet, INT, "get mismatch - expected 0");

	SlotArray.free(slarr);
}
/* Test remove item */
void test_remove_item(void)
{
	fprintf(stdout, "\n");
	fflush(stdout);

	int cap = 5;
	int expCount = 0;

	slot_array slarr = SlotArray.new(cap);
	object item = (object)0x12345678;

	SlotArray.add(slarr, item);
	SlotArray.remove(slarr, item);

	int actCount = SlotArray.count(slarr);

	Assert.areEqual(&expCount, &actCount, INT, "count mismatch");

	SlotArray.free(slarr);
}
/* Test get at invalid index - return false */
void test_getAtIndex_invalid(void)
{
	fprintf(stdout, "\n");
	fflush(stdout);

	int cap = 5;
	int expGet = 0;

	slot_array slarr = SlotArray.new(cap);
	object item = (object)0x12345678;

	SlotArray.add(slarr, item);

	object actValue;
	int actGet = SlotArray.tryGetAt(slarr, 10, &actValue);

	Assert.areEqual(&expGet, &actGet, INT, "get mismatch - expected 0");

	SlotArray.free(slarr);
}
/* Test dynamic resize */
void test_dynamic_resize(void)
{
	fprintf(stdout, "\n");
	fflush(stdout);

	int cap = 5;
	int expCount = 6;
	int expCap = 10; // dynamic resize cap*2

	slot_array slarr = SlotArray.new(cap);
	object item = (object)0x12345678;

	// will force resize
	for (int i = 0; i < cap + 1; i++)
	{
		SlotArray.add(slarr, item);
	}

	int actCount = SlotArray.count(slarr);
	int actCap = SlotArray.capacity(slarr);

	Assert.areEqual(&expCount, &actCount, INT, "count mismatch");
	Assert.areEqual(&expCap, &actCap, INT, "capacity mismatch");

	SlotArray.free(slarr);
}
/* test slot iterator with find next */
void test_slot_iterator(void)
{
	fprintf(stdout, "\n");
	fflush(stdout);

	int cap = 5;
	int expCount = 2;
	int key1 = 42, key2 = 43, key3 = 74;

	slot_array slarr = SlotArray.new(cap);
	SlotArray.add(slarr, &key1);
	SlotArray.add(slarr, &key2);
	SlotArray.add(slarr, &key3);
	SlotArray.remove(slarr, &key2);

	iterator it = Array.getIterator(slarr, SLOT);
	Assert.isTrue(it != NULL, "iterator allocation failed");

	int iterCount = 0;
	object expected[] = {&key1, &key3};
	object item;
	while (Iterator.findNext(it, filter_nonempty, &item))
	{
		Assert.isTrue(item == expected[iterCount], "iterator item mismatch");
		iterCount++;
	}
	Assert.areEqual(&expCount, &iterCount, INT, "iterated count mismatch");

	Iterator.free(it);
	SlotArray.free(slarr);
}

// Register test cases
__attribute__((constructor)) void init_sigtest_tests(void)
{
	register_test("slot_array_tests", slot_array_tests);
	register_test("test_new_index", test_new_index);
	register_test("test_add_item", test_add_item);
	register_test("test_getItem_pass", test_getItem_pass);
	register_test("test_getItem_fail", test_getItem_fail);
	register_test("test_remove_item", test_remove_item);
	register_test("test_getAtIndex_invalid", test_getAtIndex_invalid);
	register_test("test_dynamic_resize", test_dynamic_resize);
	register_test("test_slot_iterator", test_slot_iterator);
}
