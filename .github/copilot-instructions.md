# SigmaCore AI Coding Guidelines

## Architecture Overview
SigmaCore is a modern C library using **interface-based architecture** with global const interface instances. All _public_ functionality is accessed through interfaces like `Memory`, `String`, `Collections`, etc. Types are opaque pointers for encapsulation.

## Core Patterns

### Interface Usage
```c
// Where it simplifies internal plumbing, use global interfaces - otherwise we can inline functions directly
object buffer = Memory.alloc(1024, false);
string result = String.concat(str1, str2);
iterator it = Collections.create_iterator(collection);

// Always dispose resources explicitly
Memory.dispose(buffer);
String.dispose(result);
Iterator.dispose(it);
```

### Type Definitions
```c
// Opaque types - never dereference or access internals
typedef struct farray_s *farray;
typedef struct list_s *list;
typedef struct string_builder_s *string_builder;

// Basic types
typedef void *object;
typedef char *string;
typedef size_t usize;
```

### Memory Management
- **Explicit ownership**: Every `alloc()` requires matching `dispose()`
- **No automatic cleanup**: Containers don't free their contents
- **Consistent naming**: `dispose()` not `free()` or `destroy()`
- **Zero dependencies**: Only C standard library

### Collection Patterns
```c
// FArray - flex-array, value semantics, high-performance
farray arr = FArray.new(100, sizeof(int));
FArray.set(arr, 0, sizeof(int), &value);

// PArray - pointer array, pointer address, reference semantics
parray parr = PArray.new(50);
PArray.set(parr, 0, (addr)ptr);

// List - dynamic growth
list lst = List.new(10);
List.add(lst, item);

// SlotArray - sparse with stable handles
slot_array sa = SlotArray.new(1000);
usize handle = SlotArray.add(sa, item);
```

### String Operations
```c
// String utilities return allocated strings - caller disposes
string duped = String.dupe("hello");
string combined = String.concat(duped, " world");

// StringBuilder for efficient concatenation
string_builder sb = StringBuilder.new(64);
StringBuilder.append(sb, "Hello");
StringBuilder.appendf(sb, " %d", 42);
string result = StringBuilder.toString(sb); // allocates final string
```

### Iterator Pattern
```c
// .NET-style iteration
collection coll = FArray.as_collection(arr, sizeof(int));
iterator it = Collections.create_iterator(coll);
while (Iterator.next(it)) {
    void *item = Iterator.current(it);
    // use item
}
Iterator.dispose(it);
Collections.dispose(coll);
```

## Build & Test Workflow

### Building
```bash
make              # Build shared library
make test         # Build and run all tests
make install      # Install to system (/usr/lib, /usr/include/sigcore)
make test_memory  # Run specific test suite
```

### VS Code Tasks
- `gcc: compile active test` - Compile current test file
- `gcc: link active test` - Link test with library objects
- Use these for iterative test development

### Testing Framework
```c
// SigmaTest patterns
void test_example(void) {
    object ptr = Memory.alloc(64, false);
    Assert.isNotNull(ptr, "Allocation should succeed");
    Memory.dispose(ptr);
}

// Test files: test/test_*.c
// Logs: logs/test_*.log
```

## Code Organization

### File Structure
- `include/sigcore/` - Public headers with interfaces
- `src/` - Implementation files
- `test/` - Test suites using SigmaTest
- `internal/` - Private headers for implementation details

### Include Patterns
```c
// For everything
#include <sigcore/core.h>

// Or individual modules
#include <sigcore/memory.h>
#include <sigcore/collections.h>
#include <sigcore/strings.h>
```

### Implementation Files
- Each interface has corresponding .c file in `src/`
- Global interface instances defined at end of implementation
- Forward declarations for internal functions
- Doxygen-style documentation comments

### File Organization Patterns

#### Header Files (.h)
```c
/*
 * SigmaCore
 * Copyright (c) 2025 David Boarman (BadKraft) and contributors
 * QuantumOverride [Q|]
 * ----------------------------------------------
 * [License text]
 * ----------------------------------------------
 * File: filename.h
 * Description: Brief description of the module
 */
#pragma once

// Includes (system first, then local)
#include <stdbool.h>
#include <stddef.h>
#include "sigcore/types.h"

// Forward struct declarations (for opaque types)
typedef struct module_name_s *module_name;

// Specific typedefs
typedef struct {
    // public struct fields
} public_struct_t;

// Public struct definitions (rare, usually opaque)
struct public_struct {
    // fields
};

// Interface definitions
typedef struct sc_module_i {
    // function pointers
} sc_module_i;

extern const sc_module_i Module;
```

#### Source Files (.c)
```c
/*
 * SigmaCore
 * Copyright (c) 2025 David Boarman (BadKraft) and contributors
 * QuantumOverride [Q|]
 * ----------------------------------------------
 * [License text]
 * ----------------------------------------------
 * File: filename.c
 * Description: Implementation of module_name
 */

// Includes
#include "sigcore/module.h"
#include "internal/module_internal.h"

// Grouped forward declarations
// API functions
static return_type api_function(param_type param);

// Helper/utility functions
static return_type helper_function(param_type param);

// Opaque struct definitions
struct module_name_s {
    // private fields
};

// API function definitions
return_type api_function(param_type param) {
    // implementation
}

// API interface definition
const sc_module_i Module = {
    .function = api_function,
    // ...
};

// Helper/utility function definitions
static return_type helper_function(param_type param) {
    // implementation
}
```

## Development Conventions

### Naming
- Functions: `InterfaceName.method_name`
- Types: `sc_type_name` for structs, `type_name` for typedefs
- Files: lowercase with underscores
- Constants: `SC_CONSTANT_NAME`

### Error Handling
- Return `NULL` for allocation failures
- Return `-1` for errors, `0` for success
- No exceptions - check return values
- Assert in debug builds for programming errors

### Coding Standards
- **No macros unless defining constants** - avoid function-like macros, control flow macros, etc.
- Use inline functions or static helpers instead of macros for code reuse

### Documentation
- Doxygen comments on all public interfaces
- `@brief` for short descriptions
- `@param` and `@return` for parameters/return values

### Function Decomposition
- **Target size**: Keep functions to 30-40 lines for readability (fits single page without scrolling)
- **Break down monolithic functions** by extracting logical units into helper functions
- **Helper functions**: Use `static` for file-local helpers, place in "Helper/utility function definitions" section
- **Naming**: Helper functions use descriptive names, often prefixed with module name

```c
// BEFORE: Monolithic function (60+ lines)
void complex_operation(object param1, usize param2) {
    // 20 lines of setup
    // 20 lines of validation  
    // 20 lines of processing
    // 20 lines of cleanup
}

// AFTER: Decomposed into focused functions
static void complex_operation_validate(object, usize);
static void complex_operation_process(object, usize);
static void complex_operation_cleanup(object);

void complex_operation(object param1, usize param2) {
    complex_operation_validate(param1, param2);
    complex_operation_process(param1, param2); 
    complex_operation_cleanup(param1);
}
// Helper function definitions
static void complex_operation_validate(object param1, usize param2) {
    // 15 lines of validation logic
}

static void complex_operation_process(object param1, usize param2) {
    // 20 lines of core processing
}

static void complex_operation_cleanup(object param1) {
    // 10 lines of cleanup
}
```

## Key Files to Reference
- `include/sigcore/core.h` - All-in-one header
- `include/sigcore/types.h` - Basic type definitions
- `src/memory.c` - Interface implementation example
- `test/test_memory.c` - Testing patterns
- `Makefile` - Build system
- `README.md` - Usage examples