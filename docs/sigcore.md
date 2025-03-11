* Generic pointer type for objects *  
`typedef void* object;`
* Unsigned integer type representing memory addresses or offsets *  
`typedef uintptr_t addr;`
* Pointer to a null-terminated character string *  
`typedef char* string;`
* Pointer to an iterator_s structure for traversing collecions *  
`typedef struct iterator_s* iterator;`
* Pointer to a string_builder_s structure for efficient string construction. *  
`typedef struct string_builder_s* string_builder;`
* Pointer to a list_s structure.	*  
`typedef struct list_s* list;`

#### typedef struct IMem
Interface for memory management in sigcore.

Provides allocation and deallocation functions for all sigcore components.  
``` c
typedef struct IMem  
   /* Allocates memory of the specified size. */  
   object (*alloc)(size_t);
   /* Frees previously allocated memory. */  
   void (*free)(object);	
```  
#### typedef struct IList
Interface for managing dynamic lists of objects.

Defines operations for creating, modifying, and querying a list of addr pointers,
leveraging Mem for memory management and Iterator for traversal.

* Iterator type for colleciton iterators *  
`typedef enum { LIST, STRB } ITER_TYPE;`

``` c
typedef struct IList
```  
#### typedef struct IString
Interface for basic string manipulation in sigcore.

Provides a clean set of utility functions for common string operations,
abstracting away low-level details. Designed for simplicity and efficiency.

``` c
typedef struct IString
```  
#### typedef struct IStringBuilder
Interface for building strings efficiently in sigcore.

Methods to append text, manage capacity, and retrieve the final string,
optimized to avoid repeated allocations.

``` c
typedef struct IStringBuilder
```  
#### typedef struct IArray
Interface for retrieving an iterator from a List or STringBuilder.

Provide the List or StringBuilder object and the iterator type:
LIST, STRB

``` c
typedef struct IArray
```  
#### typedef struct IIterator
Interface for iterating over collections.

Provides methods to traverse a sequence of addr values, used by List and other structures.

``` c
typedef struct IIterator
```  
