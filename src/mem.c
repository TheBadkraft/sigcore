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
#include <stdlib.h>

const size_t DEFAULT_SIZE = 256;

struct mem_s {
    addr* block;
    addr end; /* Past last usable slot */
} singleton_mem = {0};

static object allocMem(size_t size) {
    object ptr = malloc(size);
    if (!ptr) return NULL;

    if (!singleton_mem.block) {  // Lazy init
        size_t initial_capacity = 10;
        singleton_mem.block = malloc(initial_capacity * ADDR_SIZE);
        if (!singleton_mem.block) {
            free(ptr);
            return NULL;
        }
        singleton_mem.end = (addr)singleton_mem.block + initial_capacity * ADDR_SIZE;
        for (int i = 0; i < initial_capacity; i++) {
            singleton_mem.block[i] = 0;
        }
    }

    int capacity = Collections.count(singleton_mem.block, singleton_mem.end);
    int freeSlot = Collections.nextEmpty(singleton_mem.block, singleton_mem.end);

    if (freeSlot == -1) { /* Full—resize */
        capacity = capacity ? capacity * 2 : 10;
        addr* new_block = realloc(singleton_mem.block, capacity * ADDR_SIZE);
        if (!new_block) {
            free(ptr);
            return NULL;
        }
        singleton_mem.block = new_block;
        singleton_mem.end = (addr)singleton_mem.block + capacity * ADDR_SIZE;
        for (int i = capacity / 2; i < capacity; i++) { /* Clear new slots */
            singleton_mem.block[i] = 0;
        }
        freeSlot = capacity / 2; /* First new slot */
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

const IMem Mem = {
    .alloc = allocMem,
    .free = freeMem
};
