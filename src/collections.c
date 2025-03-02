// collections.c
/*
 * Provides utility functions for managing collections of addr pointers in sigcore.
 * Implements the ICollections interface with helper functions for common operations on addr arrays,
 * such as counting, copying, clearing, indexing, and compacting. Also includes IIterator support for
 * traversing addr sequences. Used by higher-level structures like List and StringBuilder to avoid
 * redundant code, ensuring consistency and efficiency across sigcore’s memory-managed components.
 */
#include "collections.h"
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

const size_t ADDR_SIZE = sizeof(addr);

//	iterator	==================================================================

/* iterator structure for traversing collections */
struct iterator_s {
	object current;
	object end;
	int step;
};

iterator create_iterator(object start, object end, int step) {
	iterator it = Mem.alloc(sizeof(struct iterator_s));
	if (it) {
		it->current = start;
		it->end = end;
		it->step = step;
	}
	
	return it;
}
/* Returns an iterator for traversing collection */
static iterator getIterator(object collection, ITER_TYPE type) {
	if (!collection) return NULL;
	
	object start = NULL;
	object end = NULL;
	int step = 0;
	
	switch (type) {
	case LIST:
		list l = (list)collection;
		if (!l->bucket) return NULL;
		start = l->bucket;
		end = (object)l->last;
		step = ADDR_SIZE;
		
		break;
	case STRB:
		string_builder sb = (string_builder)collection;
		if (!sb->buffer) return NULL;
		start = sb->buffer;
		end = (char*)sb->buffer + StringBuilder.length(sb);
		step = 1;
		
		break;
	default:
		return NULL;
	}
	
	return create_iterator(start, end, step);
}
/* Returns the next item in the iteration, or 0 if at the end */
static object getNext(iterator it) {
	if (!it || it->current >= it->end) return NULL;
	object value = (object)*(addr*)it->current;
	it->current = (char*)it->current + it->step;
	
	return value;
}
/* Returns 1 if there are more items to iterate, 0 otherwise */
static int hasNext(iterator it) {
	if (!it) return 0;
	return it->current < it->end;
}
/* Frees the iterator structure */
static void freeIterator(iterator it) {
	if (it) Mem.free(it);
}

//	bytearray		================================================================
/* Clears byte array */
static void clearByteArray(object start, size_t bytes) {
	if (!start || bytes <= 0) return;
	memset(start, 0, bytes);
}
/* Copies byte array to destination */
static void copyByteArrayTo(object source, object dest, size_t bytes) {
	if(!source || !dest) return;
	memcpy(dest, source, bytes);
}
//	collections	================================================================
/* Returns the number of addr elements between start and end */
static int countSpan(addr* start, addr end) {
	if (!start) return 0;
	return (end - (addr)start) / ADDR_SIZE;
}
/* Sets all elements between start and end to 0 */
static void clearCollection(addr* start, addr end) {
	if (!start) return;
	addr* pS = start;
	while ((addr)pS != end) {
		*pS++ = 0;
	}
}
/* Copies elements from source to dest up to the end address */
static void copyCollectionTo(addr* source, addr* dest, addr end) {
	if (!source || !dest) return;
	addr* pS = source;
	addr* pD = dest;
	while ((addr)pS != end) {
		*pD++ = *pS++;
	}
}
/* Returns the index of the first occurrence of item, or -1 if not found */
static int getIndex(addr* start, addr end, object item) {
	if (!start) return -1;
	//printf("\niterator looking for %p\n", item);
	
	iterator it = create_iterator(start, (object)end, ADDR_SIZE);
	int i = 0;
	object current = NULL;
	while(hasNext(it)) {
		current = getNext(it);
		//printf("iterator[%d] -> %p\n", i, current);
		
		if (current == item) {
			freeIterator(it);
			return i;
		}
		
		i++;
	}
	
	freeIterator(it);
	return -1;
}
/* Returns the item at the specified index, or 0 if out of bounds */
static addr getAtIndex(addr* start, addr end, int index) {
	if (!start) return 0;
	if (index < 0 || index >= countSpan(start, end)) return 0;
	return start[index];
}
/* Sets the item at the specified index to 0, if within bounds */
static void removeItemAt(addr* start, addr end, int index) {
	if (!start) return;
	if (index < 0 || index >= countSpan(start, end)) return;
	start[index] = 0;
}
/* Shifts non-zero elements left and updates end, clearing remaining slots */
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
/* Returns the index of the first empty (0) slot, or -1 if none */
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

const IByteArray ByteArray = {
	.clear = clearByteArray,
	.copyTo = copyByteArrayTo,
};

const ICollections Collections = {
	.count = countSpan,
	.clear = clearCollection,
	.copyTo = copyCollectionTo,
	.indexOf = getIndex,
	.getAtIndex = getAtIndex,
	.removeAtIndex = removeItemAt,
	.compact = compactCollection,
	.nextEmpty = getNextEmpty
};

const IArray Array = {
	.getIterator = getIterator
};

const IIterator Iterator = {
	.next = getNext,
	.hasNext = hasNext,
	.free = freeIterator
};
