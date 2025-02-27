// collections.c
#include "collections.h"
#include <stdlib.h>

#include <stdio.h>

const size_t ADDR_SIZE = sizeof(addr);

//	iterator	==================================================================
static iterator getIterator(addr* start, addr end) {
	iterator it = Mem.alloc(sizeof(struct iterator_s));
	if (it) {
		it->current = start;
		it->end = end;
	}
	
	return it;
}

static addr getNext(iterator it) {
	if (!it || (addr)it->current >= it->end) return 0;
	addr value = *it->current;
	it->current++;
	
	return value;
}

static int hasNext(iterator it) {
	if (!it) return 0;
	return (addr)it->current < it->end;
}

static void freeIterator(iterator it) {
	if (it) Mem.free(it);
}

//	collections	================================================================
static int getCount(addr* start, addr end) {
	if (!start) return 0;
	return (end - (addr)start) / ADDR_SIZE;
}


static void clearCollection(addr* start, addr end) {
	if (!start) return;
	addr* pS = start;
	while ((addr)pS != end) {
		*pS++ = 0;
	}
}

static void copyCollectionTo(addr* source, addr* dest, addr end) {
	if (!source || !dest) return;
	addr* pS = source;
	addr* pD = dest;
	while ((addr)pS != end) {
		*pD++ = *pS++;
	}
}

static int getIndex(addr* start, addr end, addr item) {
	if (!start) return -1;

	iterator it = getIterator(start, end);
	int i = 0;
	while(hasNext(it)) {
		if (getNext(it) == item) {
			freeIterator(it);
			return i;
		}
		
		i++;
	}
	
	freeIterator(it);
	return -1;
}

static addr getItemAt(addr* start, addr end, int index) {
	if (!start) return 0;
	if (index < 0 || index >= getCount(start, end)) return 0;
	return start[index];
}

static void removeItemAt(addr* start, addr end, int index) {
	if (!start) return;
	if (index < 0 || index >= getCount(start, end)) return;
	start[index] = 0;
}

static void compactCollection(addr* start, addr* end) {
	if (!start || !end) return;
	int offset = 0;
	addr* pS = start;
	
	//	shift non-empties left
	while ((addr)pS != *end) {
		if (*pS != 0) { start[offset++] = *pS; }
		++pS;
	}
	
	//	empty remaining slots
	clearCollection(start + offset, *end);
	
	*end = (addr)start + (offset * ADDR_SIZE);
}

static int getNextEmpty(addr* start, addr end) {
	if (!start) return -1;
	int i = 0;
	addr *pS = start;
	
	while ((addr)pS != end) {
		if (*pS == 0) return i;
		pS++;
		i++;
	}
	
	return -1;
}


const ICollections Collections = {
	.count = getCount,
	.capacity = getCount,
	.clear = clearCollection,
	.copyTo = copyCollectionTo,
	.indexOf = getIndex,
	.getAtIndex = getItemAt,
	.removeAtIndex = removeItemAt,
	.compact = compactCollection,
	.nextEmpty = getNextEmpty,
	.iterator = getIterator
};

const IIterator Iterator = {
	.next = getNext,
	.hasNext = hasNext,
	.free = freeIterator
};
