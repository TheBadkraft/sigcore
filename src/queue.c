// queue.c
#include "sigcore.h"
#include "collections.h"
#include <stdlib.h>

//	Function declarations ======================================================
static int isQueueEmpty(queue);
static void resizeQueue(queue);
static int isQueueFull(queue);

/* Creates a new queue with initial capacity, allocating space for capacity + 1 slots */
static queue newQueue(int capacity) {
	queue q = Mem.alloc(sizeof(struct queue_s));
	if (!q) return NULL;
	
	q->bucket = Mem.alloc(capacity * ADDR_SIZE);
	if (!q->bucket) {
		Mem.free(q);
		return NULL;
	}
	
	q->first = (addr)q->bucket;
	q->last = (addr)q->bucket;
	q->end = (addr)q->bucket + (capacity * ADDR_SIZE);
	Collections.clear(q->bucket, q->end);
	
	return q;
}
/* Frees memory allocation for a queue */
static void freeQueue(queue q) {
	if (q) {
		if (q->bucket) Mem.free(q->bucket);
		Mem.free(q);
	}
}
/* Enqueue item */
static void enqueueItem(queue q, object item) {
	if (!q) return;
	
	if (isQueueFull(q)) {
		resizeQueue(q);
		if (isQueueFull(q)) return;  // Resize failed, drop item
	}
	
	*(addr*)q->last = (addr)item;
	q->last += ADDR_SIZE;
	if (q->last == q->end) q->last = (addr)q->bucket;	// circular queue
}
/* Dequeue item */
static object dequeueItem(queue q) {
    if (!q || isQueueEmpty(q)) return NULL;
    
    object item = (object)*(addr*)q->first;
    *(addr*)q->first = 0;  // clear the slot
    q->first += ADDR_SIZE;	//	advance to the next in front
    if (q->first == q->end) q->first = (addr)q->bucket;
    
    return item;
}
/* Peek at front item */
static object peekItem(queue q) {
    if (!q || isQueueEmpty(q)) return NULL;
    return (object)*(addr*)q->first;
}
/* Get the queue's capacity */
static int getCapacity(queue q) {
	if (!q) return 0;
	return (q->end - (addr)q->bucket) / ADDR_SIZE;
}
/* Clear the queue */
static void clearQueue(queue q) {
	if (!q) return;
	
	Collections.clear(q->bucket, q->end);
	q->first = (addr)q->bucket;
	q->last = (addr)q->bucket;
}
/* Get the queue's enqueued count */
static int getCount(queue q) {
	if (!q) return 0;
	if (q->last >= q->first) {
		return (q->last - q->first) / ADDR_SIZE;
	} else {
		return ((q->end - q->first) + (q->last - (addr)q->bucket)) / ADDR_SIZE;
	}
}
/* determine if the queue is empty */
static int isQueueEmpty(queue q) {
    if (!q) return 1;
    return q->first == q->last;
}
/* determine if the queue is full */
static int isQueueFull(queue q) {
    if (!q) return 0;
    addr nextLast = q->last + ADDR_SIZE;
    if (nextLast == q->end) nextLast = (addr)q->bucket;
    return nextLast == q->first;
}

/* Resizes the queue by doubling its capacity */
static void resizeQueue(queue q) {
    if (!q) return;
    
    int count = getCount(q);
    int oldCapacity = getCapacity(q);
    int newCapacity = oldCapacity * 2;
    addr* newBucket = Mem.alloc(newCapacity * ADDR_SIZE);
    if (!newBucket) return;  // Fail silently, enqueue will retry
    
    // Copy items linearly from first to last
    int i = 0;
    addr pos = q->first;
    while (i < count) {
        newBucket[i] = *(addr*)pos;
        pos += ADDR_SIZE;
        if (pos == q->end) pos = (addr)q->bucket;
        i++;
    }
    
    // Clear remaining slots
    Collections.clear(newBucket + count, (addr)newBucket + newCapacity * ADDR_SIZE);
    
    Mem.free(q->bucket);
    q->bucket = newBucket;
    q->first = (addr)q->bucket;
    q->last = (addr)q->bucket + count * ADDR_SIZE;
    q->end = (addr)q->bucket + newCapacity * ADDR_SIZE;
}

const IQueue Queue = {
	.new = newQueue,
	.free = freeQueue,
	.enqueue = enqueueItem,
	.dequeue = dequeueItem,
	.peek = peekItem,
	.count = getCount,
	.capacity = getCapacity,
	.clear = clearQueue,
	.isEmpty = isQueueEmpty,
	.isFull = isQueueFull
};
