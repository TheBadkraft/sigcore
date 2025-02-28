// list.c
/*
 * Implements a dynamic list of object pointers for sigcore.
 * Provides the IList interface for managing a growable array of addr (object pointers).
 * Uses Mem for allocation, with bucket as the storage array, end as the address of the last used element
 * (indicating the next slot is free), and end pointing past the last usable slot. Resizes by doubling capacity
 * when full, leveraging Collections for copy, clear, and index operations. Designed for efficient addition
 * and iteration, with a focus on modularity and reuse across sigcore applications.
 */
#include "sigcore.h"
#include "collections.h"
#include <stdlib.h>

#include <stdio.h>

//	static void print_list_items(list);
static int getCount(list self);
static int getCapacity(list self);

/* Creates a new list with initial capacity, allocating space for capacity + 1 slots */
static list newList(int capacity) {
	//printf("create new list cap=%d ", capacity);
	list self = Mem.alloc(sizeof(struct list_s));
	//printf("[%s]\n", self != NULL ? "TRUE" : "FALSE");
	
	if (self) {
		self->bucket = Mem.alloc((capacity + 1) * ADDR_SIZE);
		
		if (!self->bucket) {
			Mem.free(self);
			return NULL;
		}
		self->last = (addr)self->bucket;
		self->end = (addr)self->bucket + capacity * ADDR_SIZE;
		
		//printf("set list last=%lu end=%lu capacity=%d\n", self->last, self->end, getCapacity(self));
		
		Collections.clear(self->bucket, self->end + ADDR_SIZE);
	}
		
	return self;
}
/* Returns the number of items currently in the list */
static int getCount(list self) {
	return Collections.count(self->bucket, self->last);
}
/* Returns the total number of slots available in the list */
static int getCapacity(list self) {
	return Collections.count(self->bucket, self->end);
}
/* Resets the list to empty, clearing all items and resetting last to the start */
static void clearList(list self) {
	if (!self) return;
	Collections.clear(self->bucket, self->last);
	self->last = (addr)self->bucket;
}
/* Returns an iterator for traversing the list’s items */
static iterator getListIterator(list self) {
	return Collections.iterator(self->bucket, self->last);
}
/* Frees the list’s bucket and the list structure itself */
static void freeList(list self) {
	//printf("\nMem.free ");
	if (self) {
		Mem.free(self->bucket);
		//printf("bucket(%p)\n", self->bucket);
		Mem.free(self);
		//printf("Mem.free self  (%p)\n", self);
	}
}
/* Adds an object to the list, resizing by doubling capacity if full */
static void addItem(list self, object item) {
	if (!self) return;
	int count = getCount(self);
	int capacity = getCapacity(self);

	//	resizing if necessary (doubling capacity)
	if (self->last == self->end) {
		capacity = capacity ? capacity * 2 : 4;
		addr* new_bucket = Mem.alloc((capacity + 1) * ADDR_SIZE);
		if (!new_bucket) return;

		//	new Collections.copyTo
		Collections.copyTo(self->bucket, new_bucket, self->last);
		//	leverage Collections.clear
		Collections.clear(new_bucket + count, (addr)new_bucket + (capacity + 1) * ADDR_SIZE);
		
		Mem.free(self->bucket);
		self->bucket = new_bucket;
		self->last = (addr)self->bucket + count * ADDR_SIZE;
		self->end = (addr)self->bucket + capacity * ADDR_SIZE;
	}
	
	self->bucket[count] = (addr)item;
	self->last += ADDR_SIZE;
}
/* Copies a range of items from source to dest, resizing dest if needed */
static int copyToList(list source, list dest, int start, int count) {
	int res = 0;
	if (!source || !dest || start < 0 || count < 0) return res;
	
	//	validate source count and capacity
	int srcCount = getCount(source);
	if(start >= srcCount) return 0;
	
	//	adjust count to fit source availability
	int itemCount = srcCount - start;
	int copyCount = (count > itemCount) ? itemCount : count;
	if (copyCount <= 0) return 0;
	
	//	validate dest count and capacity
	int dstCount = getCount(dest);
	int dstCapacity = getCapacity(dest);
	int incrCapacity = dstCount + copyCount;
	
	//	resize if necessary
	if (incrCapacity > dstCapacity) {
		int newCapacity = dstCapacity ? dstCapacity * 2 : 4;
		while (newCapacity < incrCapacity) newCapacity *= 2;
		
		addr* new_bucket = Mem.alloc((newCapacity + 1) * ADDR_SIZE);
		if (!new_bucket) return 0;
		
		//	copy existing dest items
		Collections.copyTo(dest->bucket, new_bucket, dest->last);
		Collections.clear(new_bucket + dstCount, (addr)new_bucket + (newCapacity + 1) * ADDR_SIZE);
		
		Mem.free(dest->bucket);
		dest->bucket = new_bucket;
		dest->last = (addr)dest->bucket + dstCount * ADDR_SIZE;
		dest->end = (addr)dest->bucket + newCapacity * ADDR_SIZE;
	}
	
	//	copy items from source to dest
	addr* srcStart = source->bucket + start;
	addr* dstStart = dest->bucket + dstCount;
	addr dstEnd = dest->last + (copyCount * ADDR_SIZE);
	Collections.copyTo(srcStart, dstStart, dstEnd);
	dest->last = dstEnd;
	
	return copyCount;
}
/* Returns the index of the first occurrence of an item, or -1 if not found */
static int getIndexOf(list self, object item) {
	int res = -1;
	if (self) {
		res = Collections.indexOf(self->bucket, self->last, (addr)item);
	}
	
	return res;
}
/* Returns the object at the specified index */
static object getItemAt(list self, int index) {
	if(!self) return NULL;
	addr item = Collections.getAtIndex(self->bucket, self->last, index);
	return (object)item;
}
/* Removes the first occurrence of an item, compacting the list afterward */
static void removeItem(list self, object item) {
	if (!self) return;
	
	int index = Collections.indexOf(self->bucket, self->last, (addr)item);
	if (index != -1) {
		//	we found the item
		Collections.removeAtIndex(self->bucket, self->last, index);
		//printf("removed:   item=%p end=%p count=%d\n", (object)item, (object)self->end, getCount(self));
		Collections.compact(self->bucket, &self->last);
		//printf("compacted:       end=%p count=%d\n", (object)self->end, getCount(self));
	}
}

/* utiliity print method */
/*
static void print_list_items(list source) {
	printf("list:   count=%d capacity=%d\n", getCount(source), getCapacity(source));
	iterator it = getListIterator(source);
	if (!it) {
		printf("error retrieving list iterator");
		return;
	}
	
	int index = 0;
	addr item;
	while(Iterator.hasNext(it)) {
		item = Iterator.next(it);
		printf("[%d] item=%p (%ld)\n", index, (object)item, item);
	}
}
*/

const IList List = {
	.new = newList,
	.free = freeList,
	.add = addItem,
	.copyTo = copyToList,
	.indexOf = getIndexOf,
	.getAt = getItemAt,
	.remove = removeItem,
	.count = getCount,
	.capacity = getCapacity,
	.clear = clearList,
	.iterator = getListIterator
};
