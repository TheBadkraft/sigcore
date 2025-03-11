#### typedef struct IMem
Interface for memory management in sigcore.

Provides allocation and deallocation functions for all sigcore components.

``` c
typedef struct IMem
```  
#### typedef struct IList
Interface for managing dynamic lists of objects.

Defines operations for creating, modifying, and querying a list of addr pointers,
leveraging Mem for memory management and Iterator for traversal.

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
