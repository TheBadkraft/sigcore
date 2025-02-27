// mem.c
#include "sigcore.h"
#include "collections.h"
#include <stdlib.h>

const size_t DEFAULT_SIZE = 256;

// Mem
struct mem_s {
	addr* bucket;
	addr last;
	addr cap;
} singleton_mem = {0};

//	TODO: refactor to reduce code duplication - getCount, getCapacity, nextSlot, etc.
static object allocMem(size_t size) {
	object ptr = malloc(size);
	if (ptr) {
		if (!singleton_mem.bucket) {  // Lazy init
			singleton_mem.bucket = malloc(10 * ADDR_SIZE);
			singleton_mem.last = (addr)singleton_mem.bucket;
			singleton_mem.cap = (addr)singleton_mem.bucket + 10 * ADDR_SIZE;
			for (int i = 0; i < 10; i++) {
				singleton_mem.bucket[i] = 0;
			}
		}
		int count = Collections.count(singleton_mem.bucket, singleton_mem.last);
		int capacity = Collections.capacity(singleton_mem.bucket, singleton_mem.cap);
		int freeSlot = Collections.nextEmpty(singleton_mem.bucket, singleton_mem.last);
		
		if (freeSlot != -1) {
			//	reuse free slot
			singleton_mem.bucket[freeSlot] = (addr)ptr;
		} else if (singleton_mem.last == singleton_mem.cap) {
			//	resize and append
			capacity = capacity ? capacity * 2 : 10;
			addr* new_bucket = realloc(singleton_mem.bucket, capacity * ADDR_SIZE);
			if (!new_bucket) {
				free(ptr);
				return NULL;
			}
			singleton_mem.bucket = new_bucket;
			singleton_mem.last = (addr)singleton_mem.bucket + count * ADDR_SIZE;
			singleton_mem.cap = (addr)singleton_mem.bucket + capacity * ADDR_SIZE;
			for (int i = count; i < capacity; i++) {
				singleton_mem.bucket[i] = 0;
			}
			singleton_mem.bucket[count] = (addr)ptr;
			singleton_mem.last += ADDR_SIZE;
		} else {
			//	append to existing space
			singleton_mem.bucket[count] = (addr)ptr;
			singleton_mem.last += ADDR_SIZE;
		}
	}
	
	return ptr;
}

static void freeMem(object ptr) {
	int count = (singleton_mem.last - (addr)singleton_mem.bucket) / ADDR_SIZE;
	for (int i = 0; i < count; i++) {
		if (singleton_mem.bucket[i] == (addr)ptr) {
			singleton_mem.bucket[i] = 0;  // Mark slot free
			break;  // First match only
		}
	}
	free(ptr);
}

const IMem Mem = {
	.alloc = allocMem,
	.free = freeMem
};
