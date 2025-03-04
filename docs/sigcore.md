### *(header)* include/sigcore.h

####  Interface for memory management in sigcore. 

Provides allocation and deallocation functions for all sigcore components.

``` c
typedef struct IMem
```
####  Interface for managing dynamic lists of objects. 

Defines operations for creating, modifying, and querying a list of addr pointers,

``` c
typedef struct IList
```
####  Interface for building strings efficiently in sigcore. 

Methods to append text, manage capacity,

``` c
typedef struct IStringBuilder
```
####  Interface for retrieving an iterator from a List or STringBuilder. 

Provide the List or StringBuilder object and the iterator type:

``` c
typedef struct IArray
```
####  Interface for iterating over collections. 

Provides methods to traverse a sequence of addr values, used by List and other structures.

``` c
typedef struct IIterator
```
