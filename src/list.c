// list.c
#include "sigcore.h"
#include "collections.h"
#include <stdlib.h>

#include <stdio.h>

static list newList(int capacity) {
	list self = Mem.alloc(sizeof(struct list_s));
	if (self) {
		self->bucket = Mem.alloc((capacity + 1) * ADDR_SIZE);
		if (!self->bucket) {
			Mem.free(self);
			return NULL;
		}
		self->last = (addr)self->bucket;
		self->cap = (addr)self->bucket + capacity * ADDR_SIZE;
		
		Collections.clear(self->bucket, self->cap + ADDR_SIZE);
	}
		
	return self;
}

static int getCount(list self) {
	return Collections.count(self->bucket, self->last);
}

static int getCapacity(list self) {
	return Collections.capacity(self->bucket, self->cap);
}

static void clearList(list self) {
	if (!self) return;
	Collections.clear(self->bucket, self->last);
	self->last = (addr)self->bucket;
}

static iterator getListIterator(list self) {
	return Collections.iterator(self->bucket, self->last);
}

static void destroyList(list self) {
	if (self) {
		Mem.free(self->bucket);
		Mem.free(self);
	}
}

static void addItem(list self, object item) {
	if (!self) return;
	int count = getCount(self);
	int capacity = getCapacity(self);

	//	resizing if necessary (doubling capacity)
	if (self->last == self->cap) {
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
		self->cap = (addr)self->bucket + capacity * ADDR_SIZE;
	}
	
	self->bucket[count] = (addr)item;
	self->last += ADDR_SIZE;
}

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
		dest->cap = (addr)dest->bucket + newCapacity * ADDR_SIZE;
	}
	
	//	copy items from source to dest
	addr* srcStart = source->bucket + start;
	addr* dstStart = dest->bucket + dstCount;
	addr dstEnd = dest->last + (copyCount * ADDR_SIZE);
	Collections.copyTo(srcStart, dstStart, dstEnd);
	dest->last = dstEnd;
	
	return copyCount;
}

static int getIndexOf(list self, object item) {
	int res = -1;
	if (self) {
		res = Collections.indexOf(self->bucket, self->last, (addr)item);
	}
	
	return res;
}

static void removeItem(list self, object item) {
	if (!self) return;
	
	int index = Collections.indexOf(self->bucket, self->last, (addr)item);
	if (index != -1) {
		//	we found the item
		Collections.removeAtIndex(self->bucket, self->last, index);
//		printf("removed:   item=%p last=%p count=%d\n", (object)item, (object)self->last, getCount(self));
		Collections.compact(self->bucket, &self->last);
//		printf("compacted:       last=%p count=%d\n", (object)self->last, getCount(self));
	}
}

const IList List = {
	.new = newList,
	.destroy = destroyList,
	.add = addItem,
	.copyTo = copyToList,
	.indexOf = getIndexOf,
//	.get = list_get,
	.remove = removeItem,
	.count = getCount,
	.capacity = getCapacity,
	.clear = clearList,
	.iterator = getListIterator
};
