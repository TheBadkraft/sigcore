// test_list.c
#include <stdio.h>
#include "sigtest.h"
#include "sigcore.h"

//	for the unit test
#include "../src/collections.h"

typedef struct person {
	int id;
	string fname;
	string lname;
} person;

void new_list(void) {
	int expCap = 5;
	int expCount = 0;
	
	list index = List.new(expCap);
	
	int actCap = List.capacity(index);
	int actCount = List.count(index);
	
	printf("\nallocate list: expected {cap(%d) count(%d)} :: ", expCap, expCount);
	Assert.isTrue(index != NULL, "list (index) did not allocate");
	Assert.areEqual(&expCap, &actCap, INT, "capacities are not equal");
	Assert.areEqual(&expCount, &actCount, INT, "couns are not equal");
	
	List.destroy(index);
}

void add_item(void) {
	int expCap = 5;
	int expCount = 1;
	
	person david = {
		.id = 1, .fname = "David", .lname = "Boarman"
	};
	
	list index = List.new(expCap);
	List.add(index, &david);
	
	int actCap = List.capacity(index);
	int actCount = List.count(index);
	
	printf("\nallocate list: expected {cap(%d) count(%d)} :: ", expCap, expCount);
	Assert.areEqual(&expCap, &actCap, INT, "capacities are not equal");
	Assert.areEqual(&expCount, &actCount, INT, "counts are not equal");
	
	List.destroy(index);
}

void item_index(void) {
	int expCap = 5;
	
		person david = {
		.id = 1, .fname = "David", .lname = "Boarman"
	};
	person mandy = {
		.id = 2, .fname = "Mandy", .lname = "Boarman"
	};
	
	list index = List.new(expCap);
	List.add(index, &david);
	List.add(index, &mandy);
	
	int ndxOf = List.indexOf(index, &david);
	printf("\n indexOf(david) :: %d", ndxOf);
	Assert.isTrue(0 == ndxOf, "indexOf(david) mismatch");
	
	ndxOf = List.indexOf(index, &mandy);
	printf("\n indexOf(mandy) :: %d", ndxOf);
	Assert.isTrue(1 == ndxOf, "indexOf(mandy) mismatch");
	
	List.destroy(index);
}

void remove_item(void) {
	int expCap = 5;
	int expCount = 0;
	
	person david = {
		.id = 1, .fname = "David", .lname = "Boarman"
	};
	person rick = {
		.id = 4, .fname = "Rick", .lname = "Blackburn"
	};
	person mandy = {
		.id = 2, .fname = "Mandy", .lname = "Boarman"
	};
	
	list index = List.new(expCap);
	List.add(index, &david);
	List.add(index, &rick);
	List.add(index, &mandy);
	expCount = List.count(index);
	
	int actCap = List.capacity(index);
	int actCount = List.count(index);
	
	printf("\nallocate list: expected {cap(%d) count(%d)} :: ", expCap, expCount);
	Assert.areEqual(&expCap, &actCap, INT, "capacities are not equal");
	Assert.areEqual(&expCount, &actCount, INT, "counts are not equal");
	
	List.remove(index, &mandy);
	--expCount;
	actCount = List.count(index);
	
	printf("\nremove item:   expected {cap(%d) count(%d)} :: ", expCap, expCount);
	Assert.areEqual(&expCount, &actCount, INT, "counts are not equal");

	List.destroy(index);
}

void clear_list(void) {
	int expCap = 5;
	int expCount = 0;

	list index = List.new(expCap);
	
	person david = {
		.id = 1, .fname = "David", .lname = "Boarman"
	};
	person mandy = {
		.id = 2, .fname = "Mandy", .lname = "Boarman"
	};	
	List.add(index, &david);
	List.add(index, &mandy);
	
	List.clear(index);
	int actCount = List.count(index);
	
	printf("\nremove item:   expected {cap(%d) count(%d)} :: ", expCap, expCount);
	Assert.areEqual(&expCount, &actCount, INT, "counts are not equal");
	
	List.destroy(index);
}

void copy_list(void) {
	list source = List.new(3);
	list dest = List.new(1);
		
	person david = {
		.id = 1, .fname = "David", .lname = "Boarman"
	};
	person rick = {
		.id = 4, .fname = "Rick", .lname = "Blackburn"
	};
	person mandy = {
		.id = 2, .fname = "Mandy", .lname = "Boarman"
	};
	person evert = {
		.id = 8, .fname = "Everett", .lname = "Hood"
	};
	
	//	source items
	List.add(source, &david);
	List.add(source, &mandy);
	List.add(source, &evert);
	
	//	dest item
	List.add(dest, &rick);
	
	//	copy
	List.copyTo(source, dest, 0, 3);
	
	int expCount = 4;
	int actCount = List.count(dest);
	
	printf("\nremove item:   expected {count(%d)} :: actual {cap(%d)}", expCount, List.capacity(dest));
	Assert.areEqual(&expCount, &actCount, INT, "counts are not equal");
	
	List.destroy(source);
	List.destroy(dest);
}

void iterate_list(void) {
	int expCap = 3;
	int expCount = 0;
	
	person david = {
		.id = 1, .fname = "David", .lname = "Boarman"
	};
	person rick = {
		.id = 4, .fname = "Rick", .lname = "Blackburn"
	};
	person mandy = {
		.id = 2, .fname = "Mandy", .lname = "Boarman"
	};
	
	list index = List.new(expCap);
	List.add(index, &david);
	List.add(index, &mandy);
	List.add(index, &rick);
	expCount = List.count(index);
	
	int actCap = List.capacity(index);
	int actCount = List.count(index);
	
	printf("\nallocate list: expected {cap(%d) count(%d)} :: actual {cap(%d) count(%d)}\n", 
		expCap, expCount, actCap, actCount);
	Assert.areEqual(&expCap, &actCap, INT, "capacities are not equal");
	Assert.areEqual(&expCount, &actCount, INT, "counts are not equal");

	//	test iterator
	iterator it = List.iterator(index);
	printf("Iterator created: %p\n", (object)it);
	Assert.isTrue(it != NULL, "iterator do not initialize");

	int iterCount = 0;
	addr expected[] = { (addr)&david, (addr)&mandy, (addr)&rick };
	addr item;
	while (Iterator.hasNext(it)) {
		printf("hasNext true, iterCount=%d, current=%p, end=%p\n", 
    	iterCount, it ? (object)it->current : NULL, it ? (object)it->end : NULL);
		item = Iterator.next(it);
		printf("next returned: %p\n", (object)item);
		
		if (iterCount >= expCount || item != expected[iterCount]) {
			printf("[FAIL] Iterator item mismatch at index %d: expected %p, got %p\n", 
				iterCount, (object)expected[iterCount], (object)item);
			Iterator.free(it);
			List.destroy(index);
			return;
		}
		
		iterCount++;		
	}
	
	// Verify all items iterated
	printf("Iterated %d items\n", iterCount);
	if (iterCount != expCount) {
		printf("[FAIL] Iterated count mismatch: expected %d, got %d\n", expCount, iterCount);
		Iterator.free(it);
		List.destroy(index);
		return;
	}
    
	// Verify no extra items
	item = Iterator.next(it);
	printf("Post-loop next: %p\n", (object)item);
	if (item != 0) {
		printf("[FAIL] Iterator returned extra item: %p\n", (object)item);
		Iterator.free(it);
		List.destroy(index);
		return;
	}
    
	printf("\niterate list: expected {count(%d)} :: [PASS]\n", expCount);


	Iterator.free(it);
	List.destroy(index);
}

// Register test cases
__attribute__((constructor)) void init_sigtest_tests(void) {
//    register_test("new_list", new_list);
//    register_test("add_item", add_item);
//    register_test("item_index", item_index);
//    register_test("remove_item", remove_item);
//    register_test("clear_list", clear_list);
//		register_test("copy_list", copy_list);
			register_test("iterate_list", iterate_list);
}
