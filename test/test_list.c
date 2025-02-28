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

/* create a new list */
void new_list(void) {
	int expEnd = 5;
	int expCount = 0;
	
	list index = List.new(expEnd);
	printf("list initialized bucket=%p last=%p end=%p\n", index->bucket, (addr*)index->last, (addr*)index->end);
	
	int actCount = List.count(index);
	printf("retrieved count:    %d\n", actCount);
	int actEnd = List.capacity(index);
	printf("retrieved capacity: %d\n", actEnd);
		
	printf("\nallocate list: expected {end(%d) count(%d)} :: ", expEnd, expCount);
	Assert.isTrue(index != NULL, "list (index) did not allocate");
	Assert.areEqual(&expEnd, &actEnd, INT, "capacities are not equal");
	Assert.areEqual(&expCount, &actCount, INT, "counts are not equal");
	
	List.free(index);
}
/* add item to list */
void add_item(void) {
	int expEnd = 5;
	int expCount = 1;
	
	person david = {
		.id = 1, .fname = "David", .lname = "Boarman"
	};
	
	list index = List.new(expEnd);
	List.add(index, &david);
	
	int actEnd = List.capacity(index);
	int actCount = List.count(index);
	
	printf("\nallocate list: expected {end(%d) count(%d)} :: ", expEnd, expCount);
	Assert.areEqual(&expEnd, &actEnd, INT, "capacities are not equal");
	Assert.areEqual(&expCount, &actCount, INT, "counts are not equal");
	
	List.free(index);
}
/* get item index */
void item_index(void) {
	int expEnd = 5;
	
		person david = {
		.id = 1, .fname = "David", .lname = "Boarman"
	};
	person mandy = {
		.id = 2, .fname = "Mandy", .lname = "Boarman"
	};
	
	list index = List.new(expEnd);
	List.add(index, &david);
	List.add(index, &mandy);
	
	int ndxOf = List.indexOf(index, &david);
	printf("\n indexOf(david) :: %d", ndxOf);
	Assert.isTrue(0 == ndxOf, "indexOf(david) mismatch");
	
	ndxOf = List.indexOf(index, &mandy);
	printf("\n indexOf(mandy) :: %d", ndxOf);
	Assert.isTrue(1 == ndxOf, "indexOf(mandy) mismatch");
	
	List.free(index);
}
/* remove item */
void remove_item(void) {
	int expEnd = 5;
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
	
	list index = List.new(expEnd);
	List.add(index, &david);
	List.add(index, &rick);
	List.add(index, &mandy);
	expCount = List.count(index);
	
	int actEnd = List.capacity(index);
	int actCount = List.count(index);
	
	printf("\nallocate list: expected {end(%d) count(%d)} :: ", expEnd, expCount);
	Assert.areEqual(&expEnd, &actEnd, INT, "capacities are not equal");
	Assert.areEqual(&expCount, &actCount, INT, "counts are not equal");
	
	List.remove(index, &mandy);
	--expCount;
	actCount = List.count(index);
	
	printf("\nremove item:   expected {end(%d) count(%d)} :: ", expEnd, expCount);
	Assert.areEqual(&expCount, &actCount, INT, "counts are not equal");

	List.free(index);
}
/* clear list contents */
void clear_list(void) {
	int expEnd = 5;
	int expCount = 0;

	list index = List.new(expEnd);
	
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
	
	printf("\nremove item:   expected {end(%d) count(%d)} :: ", expEnd, expCount);
	Assert.areEqual(&expCount, &actCount, INT, "counts are not equal");
	
	List.free(index);
}
/* copy list source to destination */
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
	
	printf("\nremove item:   expected {count(%d)} :: actual {end(%d)}", expCount, List.capacity(dest));
	Assert.areEqual(&expCount, &actCount, INT, "counts are not equal");
	
	List.free(source);
	List.free(dest);
}
/* get item at index */
void get_item(void) {
	list index = List.new(5);
		
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

	List.add(index, &david);
	List.add(index, &mandy);
	List.add(index, &evert);
	List.add(index, &rick);
		
	int actEnd = List.capacity(index);
	int actCount = List.count(index);
	
	printf("\nallocate list: expected {end(%d) count(%d)}", actEnd, actCount);
	Assert.isTrue(4 == actCount, NULL);

	object pActual = List.getAt(index, 1);	//	should retrieve &mandy
	Assert.isTrue(pActual != NULL, "item (mandy) not retrieved");
	
	person actMandy = *(person*)pActual;
	Assert.isTrue((addr)&mandy == (addr)pActual, "mandy addr mismatch"); 
	Assert.isTrue(mandy.id == actMandy.id, "mandy id not the same");
	printf("\nfound {%s, %s} :: ", actMandy.lname, actMandy.fname);
		
	List.free(index);
}
/* iterate list */
void iterate_list(void) {
	int expEnd = 3;
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
	
	list index = List.new(expEnd);
	List.add(index, &david);
	List.add(index, &mandy);
	List.add(index, &rick);
	expCount = List.count(index);
	
	int actEnd = List.capacity(index);
	int actCount = List.count(index);
	
	printf("\nallocate list: expected {end(%d) count(%d)} :: actual {end(%d) count(%d)}\n", 
		expEnd, expCount, actEnd, actCount);
	Assert.areEqual(&expEnd, &actEnd, INT, "capacities are not equal");
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
			List.free(index);
			return;
		}
		
		iterCount++;		
	}
	
	// Verify all items iterated
	printf("Iterated %d items\n", iterCount);
	if (iterCount != expCount) {
		printf("[FAIL] Iterated count mismatch: expected %d, got %d\n", expCount, iterCount);
		Iterator.free(it);
		List.free(index);
		return;
	}
    
	// Verify no extra items
	item = Iterator.next(it);
	printf("Post-loop next: %p\n", (object)item);
	if (item != 0) {
		printf("[FAIL] Iterator returned extra item: %p\n", (object)item);
		Iterator.free(it);
		List.free(index);
		return;
	}
    
	printf("\niterate list: expected {count(%d)} :: [PASS]\n", expCount);


	Iterator.free(it);
	List.free(index);
}

// Register test cases
__attribute__((constructor)) void init_sigtest_tests(void) {
	register_test("new_list", new_list);
	register_test("add_item", add_item);
	register_test("item_index", item_index);
	register_test("remove_item", remove_item);
	register_test("clear_list", clear_list);
	register_test("copy_list", copy_list);
	register_test("get_item", get_item);
	register_test("iterate_list", iterate_list);
}
