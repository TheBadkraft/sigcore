# SigmaCore

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C Standard](https://img.shields.io/badge/C-C2x-blue.svg)](https://en.wikipedia.org/wiki/C2x)
[![Tests](https://img.shields.io/badge/tests-93%20passing-green.svg)](#testing)

A modern, high-performance C library providing essential utilities for memory management, dynamic collections, and string manipulation. Designed with interface-based architecture for clean, discoverable APIs and optimal performance.

## Features

- Memory Management: Centralized allocation with automatic leak detection
- Dynamic Collections: High-performance arrays, lists, and sparse arrays
- String Utilities: Efficient string building with farray-backed buffers
- Iterators: .NET-style iteration across all collection types
- Performance: Microsecond-level operations with zero-copy where possible
- Memory Safe: Comprehensive bounds checking and null safety
- Zero Dependencies: Pure C standard library only
- Interface-Based: Clean, object-oriented API design in C

## Installation

### Quick Start
```bash
git clone https://github.com/TheBadkraft/sigma-core.git
cd sigma-core
make && sudo make install
```

### Manual Build
```bash
# Build shared library
make

# Install to system (requires sudo)
sudo make install

# Build tests (requires SigmaTest)
make test
```

## Usage

SigmaCore uses a consistent interface-based design. All functionality is accessed through global interface instances.

### Memory Management

```c
#include <sigcore/memory.h>

// Allocate and manage memory
object buffer = Memory.alloc(1024, false);  // 1KB uninitialized
object zeroed = Memory.alloc(512, true);    // 512B zero-initialized

// Always dispose when done
Memory.dispose(buffer);
Memory.dispose(zeroed);
```

### String Operations

```c
#include <sigcore/strings.h>

// String utilities
string greeting = String.dupe("Hello, World!");
string name = String.dupe("SigmaCore");
string message = String.concat(greeting, name);

printf("%s\n", message);  // "Hello, World!SigmaCore"

// Convert to mutable char array
char *mutable = String.to_array(message);

// Clean up
String.dispose(greeting);
String.dispose(name);
String.dispose(message);
Memory.dispose(mutable);  // to_array allocates new memory
```

### String Building

```c
#include <sigcore/strings.h>

// Efficient string building with farray-backed buffer
string_builder sb = StringBuilder.new(64);

StringBuilder.append(sb, "Hello");
StringBuilder.appendf(sb, " %s", "World");
StringBuilder.appendl(sb, "!");  // With newline

// Get result (caller owns the string)
string result = StringBuilder.toString(sb);
printf("%s\n", result);  // "Hello World!\n"

// Stream output
StringBuilder.toStream(sb, stdout);

// Clean up
String.dispose(result);
StringBuilder.dispose(sb);
```

### Collections & Iteration

```c
#include <sigcore/collections.h>
#include <sigcore/farray.h>

// Create a high-performance fixed array
farray arr = FArray.new(10, sizeof(int));

// Add some data
int values[] = {1, 2, 3, 4, 5};
for (int i = 0; i < 5; i++) {
    FArray.set(arr, i, sizeof(int), &values[i]);
}

// Create collection view for iteration
collection coll = FArray.as_collection(arr, sizeof(int));

// Iterate using .NET-style pattern
iterator it = Collections.create_iterator(coll);
while (Iterator.next(it)) {
    int *value = (int *)Iterator.current(it);
    printf("%d ", *value);
}
printf("\n");

// Clean up
Iterator.dispose(it);
Collections.dispose(coll);
FArray.dispose(arr);
```

### Advanced Collections

```c
#include <sigcore/list.h>
#include <sigcore/slotarray.h>

// Dynamic list
list my_list = List.new(10);
List.add(my_list, &some_object);
object item = List.get(my_list, 0);

// Sparse array with stable indices
slot_array sparse = SlotArray.new(100);
usize handle = SlotArray.add(sparse, &another_object);
object retrieved = SlotArray.get(sparse, handle);

// Clean up
List.dispose(my_list);
SlotArray.dispose(sparse);
```

## Architecture

SigmaCore follows a unique interface-based architecture that brings object-oriented design principles to C:

### Interface Pattern
```c
// Global interface instances provide consistent APIs
extern const sc_memory_i Memory;
extern const sc_string_i String;
extern const sc_stringbuilder_i StringBuilder;
extern const sc_collections_i Collections;
extern const sc_iterator_i Iterator;
```

### Opaque Types
All complex types are opaque, ensuring encapsulation:
```c
typedef struct string_builder_s *string_builder;  // Opaque
typedef struct farray_s *farray;                   // Opaque
typedef struct iterator_s *iterator;               // Opaque
```

### Memory Management
- **Explicit ownership**: Clear dispose semantics
- **Leak detection**: All tests verify clean memory usage
- **Consistent naming**: `dispose()` across all types

## Testing

SigmaCore maintains comprehensive test coverage with **93 automated tests** across all modules:

- **Memory**: Allocation, deallocation, leak detection
- **Strings**: Copy, concat, format, to_array conversion
- **StringBuilder**: Append, format, capacity management
- **Collections**: Lists, arrays, sparse arrays, iteration
- **Iterators**: .NET-style next/current pattern

```bash
# Run all tests
make test

# Run specific test suite
make test_strings
make test_stringbuilder
make test_iterator

# View test output
cat logs/test_*.log
```

## Performance

SigmaCore prioritizes performance with microsecond-level operations:

- **StringBuilder append**: ~2μs (farray-backed for optimal growth)
- **Collection iteration**: ~0.5μs per element
- **Memory operations**: ~1μs allocations
- **Zero-copy operations**: Where possible to avoid unnecessary allocations

## Contributing

We welcome contributions! Please follow these guidelines:

### Development Setup
```bash
git clone https://github.com/TheBadkraft/sigma-core.git
cd sigma-core
make test  # Ensure everything works
```

### Code Style
- Use C2x standard features where appropriate
- Follow existing naming conventions (`dispose`, not `free`)
- Add comprehensive tests for new features
- Document interfaces with Doxygen-style comments
- Maintain the interface-based architecture

### Pull Request Process
1. Fork the repository
2. Create a feature branch
3. Add tests for new functionality
4. Ensure all tests pass (`make test`)
5. Update documentation as needed
6. Submit a pull request with a clear description

## License

SigmaCore is licensed under the **MIT License**. See [LICENSE](LICENSE) for details.

## Acknowledgments

SigmaCore represents a groundbreaking collaboration between human engineering expertise and AI capabilities, demonstrating the future of software development.

**The Human Element**: David Boarman (BadKraft) engineered the foundational library architecture, established the interface-based design pattern, and guided development through a rigorous Test-Driven Development (TDD) approach. His vision for modern C development and deep understanding of software engineering principles provided the creative direction and quality assurance that ensures production-ready code.

**The AI Partnership**: GitHub Copilot contributed its vast knowledge base and rapid code generation capabilities, transforming days of development work into hours of efficient implementation. Through this collaboration, features like the farray-backed StringBuilder, .NET-style iterators, and comprehensive string utilities were developed with speed, accuracy, and extensive test coverage.

**The Result**: Together, we created more than the sum of our parts - a library that combines human creativity and abstract thinking with AI's knowledge depth and implementation speed. This partnership model, where AI accelerates development while human expertise ensures quality and vision, represents the future of software engineering.

Special thanks to the [SigmaTest](https://github.com/Quantum-Override/sigma-test) framework for enabling comprehensive testing throughout development.
