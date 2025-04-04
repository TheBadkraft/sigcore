// mem.c
/*
 * Implements a memory allocation tracker for sigcore.
 * Provides the IMem interface for allocating and freeing memory using standard malloc/free,
 * tracking allocations in a growable array of addr pointers. Uses a singleton struct with
 * block as the storage array and end pointing past the last usable slot. Resizes by doubling
 * capacity when full, leveraging Collections for slot management. Designed for lightweight
 * memory tracking in sigcore without compaction or active count.
 */
#include "sigcore.h"
#include "collections.h"
#include "mem_utils.h"
#include <string.h>
#include <stdlib.h>

const size_t DEFAULT_SIZE = 256;

static int resize_memblock(int*);

struct mem_s {
	addr* block;
	addr end; /* Past last usable slot */
} singleton_mem = {0};

static void initMem(void) __attribute__((constructor));
static void initMem(void) {
	singleton_mem.block = malloc(DEFAULT_SIZE * ADDR_SIZE);
	if (singleton_mem.block) {
		singleton_mem.end = (addr)singleton_mem.block + DEFAULT_SIZE * ADDR_SIZE;
		memset(singleton_mem.block, 0, DEFAULT_SIZE * ADDR_SIZE);  // Zero-init
	} else {
		singleton_mem.end = 0;
	}
}

static object allocMem(size_t size) {
	if (!singleton_mem.block) return NULL;
	
	object ptr = malloc(size);
	if (!ptr) return NULL;

	int freeSlot = Collections.nextEmpty(singleton_mem.block, singleton_mem.end);

	if (freeSlot == -1 && !resize_memblock(&freeSlot)) { /* Full—resize */
		free(ptr);
		return NULL;
	}

	singleton_mem.block[freeSlot] = (addr)ptr;
	return ptr;
}

static void freeMem(object ptr) {
	if (!ptr || !singleton_mem.block) return;
	int capacity = (singleton_mem.end - (addr)singleton_mem.block) / ADDR_SIZE;
	for (int i = 0; i < capacity; i++) {
		if (singleton_mem.block[i] == (addr)ptr) {
			singleton_mem.block[i] = 0;
			break;
		}
	}
	free(ptr);
}

int trackMem(object ptr) {
	if (!ptr || !singleton_mem.block) return 0;

	int freeSlot = Collections.nextEmpty(singleton_mem.block, singleton_mem.end);

	if (freeSlot == -1 && !resize_memblock(&freeSlot)) return 0;

	singleton_mem.block[freeSlot] = (addr)ptr;
	return 1;
}
static int resize_memblock(int *freeSlot) {
	int old_cap = Collections.count(singleton_mem.block, singleton_mem.end);
	int capacity = old_cap ? old_cap * 2 : DEFAULT_SIZE;

	addr* new_block = realloc(singleton_mem.block, capacity * ADDR_SIZE);
	if (!new_block) return 0;

	singleton_mem.block = new_block;
	singleton_mem.end = (addr)singleton_mem.block + capacity * ADDR_SIZE;
	memset(singleton_mem.block + old_cap, 0, (capacity - old_cap) * ADDR_SIZE);
	*freeSlot = Collections.nextEmpty(singleton_mem.block, singleton_mem.end);
	
	return 1;
}

const IMem Mem = {
	.alloc = allocMem,
	.free = freeMem
};
