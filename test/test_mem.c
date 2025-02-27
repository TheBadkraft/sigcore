// test_mem.c
#include <sigtest.h>
#include "sigcore.h"

typedef struct person {
	int id;
	string fname;
	string lname;
	int age;
} person;

void test_allocate(void) {
	//	allocate person*
	person* p = Mem.alloc(sizeof(person));
	
	Assert.isTrue(p != NULL, "person failed to allocate");
	Mem.free(p);
}

//	register test cases
__attribute__((constructor)) void init_sigtest_tests(void) {
	register_test("test_allocate", test_allocate);
}
