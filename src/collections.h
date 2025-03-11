// collections.h
/*
 *	An internal header for collections utilities
 */
#ifndef COLLECTIONS_H
#define COLLECTIONS_H

#include "sigcore.h"

/* assign constant size of (uintptr_t) */
extern const size_t ADDR_SIZE;  			// declaration only

iterator create_iterator(object start, object end, int step); /* Internal */

/* opaque list structure */
struct list_s {
	addr* bucket;		/*	Array of address pointers.	*/
	addr last;			/*	Address of the last used element.	*/
	addr end;				/*	Address setting boundary of usable elements	*/
};
/* opaque string builder structure */
struct string_builder_s {
	char* buffer;
	addr last;
	addr end;
};

/* ByteArray interface */
typedef struct IByteArray {
	void (*clear)(object, size_t);
	void (*copyTo)(object, object, size_t);
} IByteArray;

/* Collections interface */
typedef struct ICollections {
	int (*count)(addr*, addr);
	void (*clear)(addr*, addr);
	void (*copyTo)(addr*, addr*, addr);		//	copy from -> to .. end
	int (*indexOf)(addr*, addr, object);
	addr (*getAtIndex)(addr*, addr, int);
	void (*removeAtIndex)(addr*, addr, int);
	void (*compact)(addr*, addr*);			//	remove all empty (0) slots
	int (*nextEmpty)(addr*, addr);			//	locate next empty (0) slot
} ICollections;

/* internal instance of the byte array interface */
extern const IByteArray ByteArray;
/* internal instance of the collections interface */
extern const ICollections Collections;

#endif // COLLECTIONS_H
