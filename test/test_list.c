/*
 *  Test File: test_list.c
 *  Description: Test cases for SigmaCore array interfaces
 */

#include "sigcore/list.h"
#include "sigcore/memory.h"
#include <sigtest/sigtest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//  let's implement a test person struct for list data
typedef struct {
   int id;
   char name[50];
   int age;
} Person;

static void load_person_list(list *);
static void dispose_persons(list);

//  configure test set
static void set_config(FILE **log_stream) {
   *log_stream = fopen("logs/test_list.log", "w");
}

//  basic initialization, disposal, and properties
static void test_list_new(void) {
   int initial_capacity = 10;
   list lst = List.new(initial_capacity);
   Assert.isNotNull(lst, "List creation failed");

   if (lst) {
      List.dispose(lst);
   }
}
static void test_list_dispose(void) {
   int initial_capacity = 10;
   list lst = List.new(initial_capacity);
   Assert.isNotNull(lst, "List creation failed");

   // spoof the list to access underlying array
   struct sc_list {
      array bucket;
      addr last;
   } *spoofed = (struct sc_list *)lst;
   // now spoof the underlying array to check disposal
   struct sc_array {
      addr *bucket;
      addr end;
   } *bucket = (struct sc_array *)spoofed->bucket;
   object allocated_bucket = (object)bucket->bucket;
   List.dispose(lst);
   // after disposal, the allocated bucket should be freed
   Assert.isFalse(Memory.has(allocated_bucket), "List disposal failed to free underlying array");
   Assert.isFalse(Memory.has(lst), "List disposal failed to free list structure");
}
static void test_list_capacity(void) {
   int exp_capacity = 20;
   list lst = List.new(exp_capacity);
   Assert.isNotNull(lst, "List creation failed");

   // spoof the list to access underlying array
   struct sc_list {
      array bucket;
      addr last;
   } *spoofed = (struct sc_list *)lst;

   int act_capacity = Array.capacity(spoofed->bucket);
   Assert.areEqual(&exp_capacity, &act_capacity, INT, "List capacity mismatch");

   List.dispose(lst);
}
static void test_list_size(void) {
   list lst = List.new(10);
   Assert.isNotNull(lst, "List creation failed");
   // size will just return the difference between
   //   last and start of bucket - 0 for now
   int act_size = List.size(lst);
   Assert.areEqual(&(int){0}, &act_size, INT, "List size mismatch");

   // maybe we will spoof the  list and bucket later
   //   to set some values and get a real size

   List.dispose(lst);
}

//  data manipulation tests
static void test_list_append_value(void) {
   // append value to list and check size increases
   list lst = List.new(5);
   // create a data record
   Person *p1 = Memory.alloc(sizeof(Person));
   p1->id = 1;
   strcpy(p1->name, "Alice");
   p1->age = 30;
   // append to list
   List.append(lst, p1);
   // check size
   int act_size = List.size(lst);
   Assert.areEqual(&(int){1}, &act_size, INT, "List size after append mismatch");

   // spoof the list to access underlying array
   struct sc_list {
      array bucket;
      addr last;
   } *spoofed = (struct sc_list *)lst;
   // now spoof the underlying array to check appended value
   struct sc_array {
      addr *bucket;
      addr end;
   } *bucket = (struct sc_array *)spoofed->bucket;
   Person *actPerson = (Person *)bucket->bucket[0];
   // just check for pointer equality
   Assert.areEqual(p1, actPerson, PTR, "List append pointer mismatch");

   Memory.free(p1);
   List.dispose(lst);
}
static void test_list_get_value(void) {
   list lst = List.new(5);
   Person *expPerson = Memory.alloc(sizeof(Person));
   expPerson->id = 1;
   strcpy(expPerson->name, "Alice");
   expPerson->age = 30;
   List.append(lst, expPerson);

   // retrieve value at index
   int index = 0;
   object retrieved = NULL;
   int result = List.get(lst, index, &retrieved);
   Assert.areEqual(&(int){0}, &result, INT, "List get failed at index %d", index);
   Assert.isNotNull(retrieved, "List get returned NULL at index %d", index);
   //  the pointers should match
   Assert.areEqual(expPerson, retrieved, PTR, "List get pointer mismatch");

   // for sanity, check retrieved data
   Person *actPerson = (Person *)retrieved;
   Assert.areEqual(&expPerson->id, &actPerson->id, INT, "List get id mismatch");
   Assert.isTrue(strcmp(expPerson->name, actPerson->name) == 0, "List get name mismatch");
   Assert.areEqual(&expPerson->age, &actPerson->age, INT, "List get age mismatch");

   Memory.free(expPerson);
   List.dispose(lst);
}
static void test_list_remove_at(void) {
   list lst = List.new(5);
   Person *expPerson = Memory.alloc(sizeof(Person));
   expPerson->id = 1;
   strcpy(expPerson->name, "Alice");
   expPerson->age = 30;
   List.append(lst, expPerson);

   // remove at index 0
   int index = 0;
   int result = List.remove(lst, index);
   Assert.areEqual(&(int){0}, &result, INT, "List remove failed at index %d", index);
   // check size is now 0
   int act_size = List.size(lst);
   Assert.areEqual(&(int){0}, &act_size, INT, "List size after remove mismatch");
   // try to get value at index 0, should fail
   object retrieved = NULL;
   result = List.get(lst, index, &retrieved);
   Assert.areEqual(&(int){-1}, &result, INT, "List get should fail after remove at index %d", index);

   Memory.free(expPerson);
   List.dispose(lst);
}
static void test_list_set_value(void) {
   list lst = List.new(5);
   Person *expP1 = Memory.alloc(sizeof(Person));
   expP1->id = 1;
   strcpy(expP1->name, "Alice");
   expP1->age = 30;
   Person *expP2 = Memory.alloc(sizeof(Person));
   expP2->id = 2;
   strcpy(expP2->name, "Bob");
   expP2->age = 25;
   // append first person
   List.append(lst, expP1);
   // set second person at index 0
   int index = 0;
   int result = List.set(lst, index, expP2);
   Assert.areEqual(&(int){0}, &result, INT, "List set failed at index %d", index);

   // retrieve value at index 0 and check if it matches expP2
   object retrieved = NULL;
   result = List.get(lst, index, &retrieved);
   Assert.areEqual(&(int){0}, &result, INT, "List get failed at index %d", index);
   Assert.areEqual(expP2, retrieved, PTR, "List set pointer mismatch at index %d", index);

   Memory.free(expP1);
   Memory.free(expP2);
   List.dispose(lst);
}
static void test_list_insert_value(void) {
   list lst = List.new(5);
   Person *expP1 = Memory.alloc(sizeof(Person));
   expP1->id = 1;
   strcpy(expP1->name, "Alice");
   expP1->age = 30;
   Person *expP2 = Memory.alloc(sizeof(Person));
   expP2->id = 2;
   strcpy(expP2->name, "Bob");
   expP2->age = 25;
   // append first person
   List.append(lst, expP1);
   // set second person at index 0
   int index = 0;
   int result = List.insert(lst, index, expP2);
   Assert.areEqual(&(int){0}, &result, INT, "List insert failed at index %d", index);

   // retrieve value at index 0 and check if it matches expP2
   object retrieved = NULL;
   result = List.get(lst, index, &retrieved);
   Assert.areEqual(&(int){0}, &result, INT, "List get failed at index %d", index);
   Assert.areEqual(expP2, retrieved, PTR, "List insert pointer mismatch at index %d", index);
   // validate object at index 1
   index = 1;
   retrieved = NULL;
   result = List.get(lst, index, &retrieved);
   Assert.areEqual(&(int){0}, &result, INT, "List get failed at index %d", index);
   Assert.areEqual(expP1, retrieved, PTR, "List insert shift pointer mismatch at index %d", index);

   Memory.free(expP1);
   Memory.free(expP2);
   List.dispose(lst);
}
static void test_list_prepend_value(void) {
   // just a convenience wrapper around insert at 0
   list lst = List.new(5);
   Person *expP1 = Memory.alloc(sizeof(Person));
   expP1->id = 1;
   strcpy(expP1->name, "Alice");
   expP1->age = 30;
   Person *expP2 = Memory.alloc(sizeof(Person));
   expP2->id = 2;
   strcpy(expP2->name, "Bob");
   expP2->age = 25;
   Person *expP3 = Memory.alloc(sizeof(Person));
   expP3->id = 3;
   strcpy(expP3->name, "Charlie");
   expP3->age = 28;
   Person *expP4 = Memory.alloc(sizeof(Person));
   expP4->id = 4;
   strcpy(expP4->name, "Diana");
   expP4->age = 32;
   Person *expP5 = Memory.alloc(sizeof(Person));
   expP5->id = 5;
   strcpy(expP5->name, "Ethan");
   expP5->age = 27;

   // append 1,2,3,4 person
   List.append(lst, expP1);
   List.append(lst, expP2);
   List.append(lst, expP3);
   List.append(lst, expP4);
   // prepend 5th person
   int result = List.prepend(lst, expP5);
   Assert.areEqual(&(int){0}, &result, INT, "List prepend failed");
   // validate 0th & 4th
   object retrieved = NULL;
   result = List.get(lst, 0, &retrieved);
   Assert.areEqual(&(int){0}, &result, INT, "List get failed at index 0");
   Assert.areEqual(expP5, retrieved, PTR, "List prepend pointer mismatch at index 0");
   result = List.get(lst, 4, &retrieved);
   Assert.areEqual(&(int){0}, &result, INT, "List get failed at index 4");
   Assert.areEqual(expP4, retrieved, PTR, "List prepend pointer mismatch at index 4");

   Memory.free(expP1);
   Memory.free(expP2);
   Memory.free(expP3);
   Memory.free(expP4);
   Memory.free(expP5);
   List.dispose(lst);
}
static void test_list_clear(void) {
   list lst;
   load_person_list(&lst);
   // clear the list by removing all elements
   int initial_size = List.size(lst);
   dispose_persons(lst);
   List.clear(lst);
   int act_size = List.size(lst);
   Assert.areEqual(&(int){5}, &initial_size, INT, "Initial list size incorrect");
   Assert.areEqual(&(int){0}, &act_size, INT, "List clear failed");

   List.dispose(lst);
}

//  advanced/bulk data manipulation tests
static void test_list_growth(void) {
   list lst;
   load_person_list(&lst);
   // loaded with 5 persons, now append 1 more to test growth (what is our growth factor?)
   Person *expP6 = Memory.alloc(sizeof(Person));
   expP6->id = 6;
   strcpy(expP6->name, "Fiona");
   expP6->age = 29;
   List.append(lst, expP6);

   // verify size is now 6
   int act_size = List.size(lst);
   Assert.areEqual(&(int){6}, &act_size, INT, "List size after growth should be 6");

   // verify capacity increased
   int act_capacity = List.capacity(lst);
   Assert.isTrue(act_capacity > 5, "List capacity should have grown beyond 5");

   dispose_persons(lst);
   List.dispose(lst);
}
static void test_list_add_all(void) {
   Assert.skip("Not implemented; low priority");
}
static void test_list_add_from_array(void) {
   Assert.skip("Not implemented; low priority");
}

//  negative & edge test cases
static void test_list_set_out_of_bounds(void) {
   list lst = List.new(5);
   Person *p = Memory.alloc(sizeof(Person));
   p->id = 1;
   strcpy(p->name, "Test");
   p->age = 25;

   // Add one element
   List.append(lst, p);

   // Try to set at invalid indices
   Person *p2 = Memory.alloc(sizeof(Person));
   p2->id = 2;
   strcpy(p2->name, "Invalid");
   p2->age = 30;

   // Index -1 (negative)
   int result = List.set(lst, -1, p2);
   Assert.areEqual(&(int){-1}, &result, INT, "List set should fail for negative index");

   // Index 1 (beyond size)
   result = List.set(lst, 1, p2);
   Assert.areEqual(&(int){-1}, &result, INT, "List set should fail for index >= size");

   // Index 5 (way beyond)
   result = List.set(lst, 5, p2);
   Assert.areEqual(&(int){-1}, &result, INT, "List set should fail for large invalid index");

   Memory.free(p);
   Memory.free(p2);
   List.dispose(lst);
}
static void test_list_get_out_of_bounds(void) {
   list lst = List.new(5);
   Person *p = Memory.alloc(sizeof(Person));
   p->id = 1;
   strcpy(p->name, "Test");
   p->age = 25;

   // Add one element
   List.append(lst, p);

   // Try to get at invalid indices
   object retrieved = NULL;

   // Index -1 (negative)
   int result = List.get(lst, -1, &retrieved);
   Assert.areEqual(&(int){-1}, &result, INT, "List get should fail for negative index");

   // Index 1 (beyond size)
   result = List.get(lst, 1, &retrieved);
   Assert.areEqual(&(int){-1}, &result, INT, "List get should fail for index >= size");

   // Index 10 (way beyond)
   result = List.get(lst, 10, &retrieved);
   Assert.areEqual(&(int){-1}, &result, INT, "List get should fail for large invalid index");

   Memory.free(p);
   List.dispose(lst);
}
static void test_list_remove_out_of_bounds(void) {
   list lst = List.new(5);
   Person *p = Memory.alloc(sizeof(Person));
   p->id = 1;
   strcpy(p->name, "Test");
   p->age = 25;

   // Add one element
   List.append(lst, p);

   // Try to remove at invalid indices

   // Index -1 (negative)
   int result = List.remove(lst, -1);
   Assert.areEqual(&(int){-1}, &result, INT, "List remove should fail for negative index");

   // Index 1 (beyond size)
   result = List.remove(lst, 1);
   Assert.areEqual(&(int){-1}, &result, INT, "List remove should fail for index >= size");

   // Index 5 (way beyond)
   result = List.remove(lst, 5);
   Assert.areEqual(&(int){-1}, &result, INT, "List remove should fail for large invalid index");

   // Verify the element is still there
   int size = List.size(lst);
   Assert.areEqual(&(int){1}, &size, INT, "List size should remain 1 after failed removes");

   Memory.free(p);
   List.dispose(lst);
}
static void test_list_append_null(void) {
   list lst = List.new(5);

   // Try to append NULL value
   int result = List.append(lst, NULL);
   Assert.areEqual(&(int){-1}, &result, INT, "List append should fail for NULL value");

   // Verify size remains 0
   int size = List.size(lst);
   Assert.areEqual(&(int){0}, &size, INT, "List size should remain 0 after failed NULL append");

   List.dispose(lst);
}
static void test_list_prepend_null(void) {
   list lst = List.new(5);

   // Try to prepend NULL value
   int result = List.prepend(lst, NULL);
   Assert.areEqual(&(int){-1}, &result, INT, "List prepend should fail for NULL value");

   // Verify size remains 0
   int size = List.size(lst);
   Assert.areEqual(&(int){0}, &size, INT, "List size should remain 0 after failed NULL prepend");

   List.dispose(lst);
}
static void test_list_set_empty_list(void) {
   list lst = List.new(5);

   // Try to set at any index on empty list
   Person *p = Memory.alloc(sizeof(Person));
   p->id = 1;
   strcpy(p->name, "Test");
   p->age = 25;

   // Index 0 (even 0 should fail on empty list)
   int result = List.set(lst, 0, p);
   Assert.areEqual(&(int){-1}, &result, INT, "List set should fail on empty list");

   // Index -1
   result = List.set(lst, -1, p);
   Assert.areEqual(&(int){-1}, &result, INT, "List set should fail for negative index on empty list");

   Memory.free(p);
   List.dispose(lst);
}
static void test_list_get_empty_list(void) {
   list lst = List.new(5);

   // Try to get from empty list
   object retrieved = NULL;

   // Index 0
   int result = List.get(lst, 0, &retrieved);
   Assert.areEqual(&(int){-1}, &result, INT, "List get should fail on empty list");

   // Index -1
   result = List.get(lst, -1, &retrieved);
   Assert.areEqual(&(int){-1}, &result, INT, "List get should fail for negative index on empty list");

   List.dispose(lst);
}
static void test_list_remove_empty_list(void) {
   list lst = List.new(5);

   // Try to remove from empty list
   int result = List.remove(lst, 0);
   Assert.areEqual(&(int){-1}, &result, INT, "List remove should fail on empty list");

   // Index -1
   result = List.remove(lst, -1);
   Assert.areEqual(&(int){-1}, &result, INT, "List remove should fail for negative index on empty list");

   List.dispose(lst);
}
static void test_list_append_null_value(void) {
   list lst = List.new(5);

   // Try to append NULL value
   int result = List.append(lst, NULL);
   Assert.areEqual(&(int){-1}, &result, INT, "List append should fail for NULL value");

   // Verify size remains 0
   int size = List.size(lst);
   Assert.areEqual(&(int){0}, &size, INT, "List size should remain 0 after failed NULL append");

   List.dispose(lst);
}

//  register test cases
__attribute__((constructor)) void init_list_tests(void) {
   testset("core_list_set", set_config, NULL);

   testcase("list_creation", test_list_new);
   testcase("list_dispose", test_list_dispose);
   testcase("list_get_capacity", test_list_capacity);
   testcase("list_get_size", test_list_size);

   testcase("list_append_value", test_list_append_value);
   testcase("list_get_value", test_list_get_value);
   testcase("list_remove_at", test_list_remove_at);
   testcase("list_set_value", test_list_set_value);
   testcase("list_insert_value", test_list_insert_value);
   testcase("list_prepend_value", test_list_prepend_value);
   testcase("list_clear", test_list_clear);

   testcase("list_growth", test_list_growth);
   testcase("list_add_all", test_list_add_all);
   testcase("list_add_from_array", test_list_add_from_array);

   testcase("list_set_out_of_bounds", test_list_set_out_of_bounds);
   testcase("list_get_out_of_bounds", test_list_get_out_of_bounds);
   testcase("list_remove_out_of_bounds", test_list_remove_out_of_bounds);
   testcase("list_append_null", test_list_append_null);
   testcase("list_prepend_null", test_list_prepend_null);
   testcase("list_set_empty_list", test_list_set_empty_list);
   testcase("list_get_empty_list", test_list_get_empty_list);
   testcase("list_remove_empty_list", test_list_remove_empty_list);
   testcase("list_append_null_value", test_list_append_null_value);
}

static void load_person_list(list *lst) {
   *lst = List.new(5);
   Person *expP1 = Memory.alloc(sizeof(Person));
   expP1->id = 1;
   strcpy(expP1->name, "Alice");
   expP1->age = 30;
   List.append(*lst, expP1);
   Person *expP2 = Memory.alloc(sizeof(Person));
   expP2->id = 2;
   strcpy(expP2->name, "Bob");
   expP2->age = 25;
   List.append(*lst, expP2);
   Person *expP3 = Memory.alloc(sizeof(Person));
   expP3->id = 3;
   strcpy(expP3->name, "Charlie");
   expP3->age = 28;
   List.append(*lst, expP3);
   Person *expP4 = Memory.alloc(sizeof(Person));
   expP4->id = 4;
   strcpy(expP4->name, "Diana");
   expP4->age = 32;
   List.append(*lst, expP4);
   Person *expP5 = Memory.alloc(sizeof(Person));
   expP5->id = 5;
   strcpy(expP5->name, "Ethan");
   expP5->age = 27;
   List.append(*lst, expP5);
}
static void dispose_persons(list lst) {
   object p = NULL;
   for (int i = 0; i < List.size(lst); i++) {
      List.get(lst, i, &p);
      // writelnf("Disposing person id: %d, name: %s", ((Person *)p)->id, ((Person *)p)->name);
      Memory.free(p);
   }
}