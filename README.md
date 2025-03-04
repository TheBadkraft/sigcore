# sigcore Library

## Overview

**sigcore** is a lightweight C library providing foundational utilities for memory management, dynamic lists, and efficient string building. It’s designed for simplicity and performance, leveraging a consistent interface for allocation, list operations, and string manipulation.

### Purpose

- **Memory Management**: Provides allocation and deallocation functions for all components.
- **Dynamic Lists**: Manages lists of `addr` pointers with operations like add, remove, and iterate.
- **String Building**: Offers optimized methods to append text and manage capacity without repeated allocations.

---

## Installation

1. **Clone the Repository**:
   ```bash
   git clone <repo-url>
   cd sigcore
Build the Library:
bash
make
Install:
bash
sudo make install
Installs libsigcore.so to /usr/lib and sigcore.h to /usr/include.

#### Usage  
**Memory Management** (`IMem`)  
Allocates and frees memory for all sigcore components.  
``` c
object ptr = Mem.alloc(1024); // Allocate 1KB
Mem.free(ptr);               // Free it
```

**Dynamic Lists** (`IList`)  
Manages a growable array of object pointers.  
``` c
list l = List.new(5);        // Create list with capacity 5
List.add(l, some_object);    // Add an object
int count = List.count(l);   // Get item count
List.free(l);                // Free list
```

**String Building** (`IStringBuilder`)  
Efficiently builds strings with minimal allocations.  
``` c
string_builder sb = StringBuilder.new(16);      // Initial capacity 16
StringBuilder.append(sb, "Hello");             // Append text
StringBuilder.appendf(sb, ", %s!", "World");   // Formatted append
string result = StringBuilder.toString(sb);    // Get final string
Mem.free(result);                              // Free result
StringBuilder.free(sb);                        // Free builder
```

Building and Testing
Build: make—creates bin/lib/libsigcore.so.
Test: make test_string_builder—runs StringBuilder tests (requires sigtest).
Clean: make clean—clears build files, keeps directories.

Contributing
Fork the repo, make changes, submit a pull request.
Use Doxygen comments in source/header files—see sigcore.h for style.

License
[Specify your license here—e.g., MIT, GPL, etc.]
