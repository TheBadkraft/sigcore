// sigcore.h
#ifndef SIGCORE_H
#define SIGCORE_H

#include <stdint.h>
#include <stddef.h>

/** @brief	Generic pointer type for objects */
typedef void* object;
/** @brief	Unsigned integer type representing memory addresses or offsets */
typedef uintptr_t addr;
/**	@brief	Pointer to a null-terminated character string */
typedef char* string;
/**	@brief	Pointer to an iterator_s structure for traversing collecions */
typedef struct iterator_s* iterator;
/** @brief 	Pointer to a string_builder_s structure for efficient string construction. */
typedef struct string_builder_s* string_builder;
/**	@brief	Pointer to a list_s structure.	*/
typedef struct list_s* list;

/** @brief Iterator type for colleciton iterators */
typedef enum { LIST, STRB } ITER_TYPE;

/**
 * @brief Interface for memory management in sigcore.
 * @details Provides allocation and deallocation functions for all sigcore components.
 */
typedef struct IMem {
object (*alloc)(size_t size);	/**< Allocates memory of the specified size. */
	void (*free)(object ptr);		/**< Frees previously allocated memory. */
} IMem;

/**
 * @brief Interface for managing dynamic lists of objects.
 * @details Defines operations for creating, modifying, and querying a list of addr pointers,
 *          leveraging Mem for memory management and Iterator for traversal.
 */
typedef struct IList {
	list (*new)(int);											/**< Creates a new list with the given capacity. */
	void (*free)(list);										/**< Destroys the list and frees its memory. */
	void (*add)(list, object);						/**< Adds an object to the list, resizing if needed. */
	int (*indexOf)(list, object);					/**< Returns the index of an object, or -1 if not found. */
	object (*getAt)(list, int);						/**< Returns the object at a specific index. */
	int (*copyTo)(list, list, int, int);	/**< Copies items from one list to another. */
	void (*remove)(list, object);					/**< Removes the first occurrence of an object. */
	int (*count)(list);										/**< Returns the number of items in the list. */
	int (*capacity)(list);								/**< Returns the current capacity of the list. */
	void (*clear)(list);									/**< Clears all items from the list. */
	iterator (*iterateRange)(list, int, int); /**< Iterate item range */
} IList;

/**
 * @brief Interface for building strings efficiently in sigcore.
 * @details Methods to append text, manage capacity,
 *          and retrieve the final string, optimized to avoid repeated allocations.
 */
typedef struct IStringBuilder {
	string_builder (*new)(size_t capacity);				/**< Initializes with a starting capacity. */
	void (*append)(string_builder, string);				/**< Appends a string to the buffer. */
	void (*appendf)(string_builder, string, ...); /**< Appends a formatted string using printf-style specifiers. */
	void (*appendLine)(string_builder, string);   /**< Appends a string followed by a newline, or just a newline if NULL. */
	void (*clear)(string_builder);								/**< Resets the buffer to empty. */
	string (*toString)(string_builder);						/**< Returns the concatenated string (caller frees). */
	size_t (*length)(string_builder);							/**< Returns the current string length. */
	size_t (*capacity)(string_builder);						/**< Returns the current capacity. */
	void (*setCapacity)(string_builder, size_t);	/**< Adjusts the capacity, preserving content. */
	void (*free)(string_builder);									/**< Frees the string builder and its buffer. */
	iterator (*iterateRange)(string_builder, int, int); /**< Iterate buffer range */
} IStringBuilder;

/**
 * @brief Interface for retrieving an iterator from a List or STringBuilder.
 * @details Provide the List or StringBuilder object and the iterator type:
 *					LIST, STRB
 */
typedef struct IArray {
	iterator (*getIterator)(object, ITER_TYPE);
} IArray;

/**
 * @brief Interface for iterating over collections.
 * @details Provides methods to traverse a sequence of addr values, used by List and other structures.
 */
typedef struct IIterator {
	object (*next)(iterator);
	int (*hasNext)(iterator);
	void (*free)(iterator);
} IIterator;

/** @brief Global instance of the memory management interface. */
extern const IMem Mem;
/** @brief Global instance of the list management interface. */
extern const IList List;
/** @brief Global instance of the string builder interface. */
extern const IStringBuilder StringBuilder;
/** @brief Global instance of the array interface */
extern const IArray Array;
/** @brief Global instance of the iterator interface. */
extern const IIterator Iterator;

#endif // SIGCORE_H
