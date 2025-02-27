// sigcore.h
#ifndef SIGCORE_H
#define SIGCORE_H

#include <stdint.h>
#include <stddef.h>

typedef void* object;
typedef uintptr_t addr;
typedef char* string;

// List
struct list_s {
	addr* bucket;
	addr last;
	addr cap;
};
typedef struct list_s* list;
typedef struct iterator_s* iterator;

//	interfaces
typedef struct IMem {
object (*alloc)(size_t size);
	void (*free)(object ptr);
} IMem;

typedef struct IList {
	list (*new)(int);
	void (*destroy)(list);
	void (*add)(list, object);
	int (*indexOf)(list, object);
	object (*get)(list, int);
	int (*copyTo)(list, list, int, int);
	void (*remove)(list, object);
	int (*count)(list);
	int (*capacity)(list);
	void (*clear)(list);
	iterator (*iterator)(list);
} IList;

//	iterator interface
typedef struct {
	addr (*next)(iterator);
	int (*hasNext)(iterator);
	void (*free)(iterator);
} IIterator;

extern const IMem Mem;
extern const IList List;
extern const IIterator Iterator;

#endif // SIGCORE_H
