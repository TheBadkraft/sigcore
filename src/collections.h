// collections.h
/*
 *	An internal header for collections utilities
 */
#ifndef COLLECTIONS_H
#define COLLECTIONS_H

#include "sigcore.h"

/* assign constant size of (uintptr_t) */
extern const size_t ADDR_SIZE;  			// declaration only

/* iterator structure for traversing collections */
struct iterator_s {
	addr* current;
	addr end;
};

/* Collections interface */
typedef struct ICollections {
	int (*count)(addr*, addr);
	void (*clear)(addr*, addr);
	void (*copyTo)(addr*, addr*, addr);	//	copy from -> to .. end
	int (*indexOf)(addr*, addr, addr);
	addr (*getAtIndex)(addr*, addr, int);
	void (*removeAtIndex)(addr*, addr, int);
	void (*compact)(addr*, addr*);			//	remove all empty (0) slots
	int (*nextEmpty)(addr*, addr);			//	locate next empty (0) slot
	iterator (*iterator)(addr*, addr);
} ICollections;

extern const ICollections Collections;

#endif // COLLECTIONS_H
