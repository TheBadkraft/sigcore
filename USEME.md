```markdown
# sigma-core

Pure C11 single-header-style core library (header + implementation in separate .c).  
Zero dependencies beyond the C standard library. No macros for control flow, no X-macros, no macro hell. Ever.

### Types

```c
typedef void*          object;     // generic object pointer
typedef uintptr_t      addr;       // address/offset
typedef char*          string;     // null-terminated
typedef struct list_s*          list;
typedef struct string_builder_s* string_builder;
typedef struct iterator_s*      iterator;
typedef struct queue_s*         queue;
typedef struct slot_array_s*    slot_array;

typedef enum { LIST, STRB, SLOT } ITER_TYPE;
```

### Global Interfaces (function tables, all const)

```c
extern const IMem           Mem;           // malloc/free wrapper
extern const IList          List;          // dynamic array of object
extern const ISlotArray     SlotArray;     // sparse slot array (stable handles)
extern const IQueue         Queue;         // circular FIFO queue
extern const IString        String;        // string utilities
extern const IStringBuilder StringBuilder; // efficient string building
extern const IArray         Array;         // get iterator from list/stringbuilder
extern const IIterator      Iterator;      // iterator operations
```

### Build

```bash
# Just include sigcore.h and compile sigcore.c
cc -std=c11 -c sigcore.c -o sigcore.o
# Link sigcore.o with your code
```

For header-only usage define `SIGCORE_IMPLEMENTATION` in exactly one .c file before including `sigcore.h`.

### Memory Management (Mem)

```c
object o = Mem.alloc(size);
Mem.free(o);
```

All other containers use `Mem` internally. Replace with custom allocator by writing two new function pointers to a new `IMem` struct and assigning it to the global `Mem` before any container creation.

### List – dynamic array (IList)

```c
list l = List.new(0);                    // initial capacity 0 → grows on demand
List.add(l, obj1);
List.add(l, obj2);
object x = List.getAt(l, 0);
int idx  = List.indexOf(l, obj2);        // -1 if not found
List.remove(l, obj2);                    // removes first occurrence
int n = List.count(l);

iterator it = Array.getIterator(l, LIST);
while (Iterator.hasNext(it))
    object item = Iterator.next(it);
Iterator.free(it);

List.free(l);                            // frees container, NOT the objects inside
```

Contents are stored as `object` (void*). User is responsible for freeing contained objects.

### SlotArray – sparse array with stable indices (ISlotArray)

Useful for entity/component systems. Returns stable integer handles.

```c
slot_array sa = SlotArray.new(64);
object obj = Mem.alloc(...);
int handle = SlotArray.add(sa, obj);     // returns stable handle, not pointer

object out;
if (SlotArray.tryGetAt(sa, handle, &out))
    ; // use out

SlotArray.remove(sa, obj);               // invalidates handle
SlotArray.free(sa);
```

### Queue – FIFO (IQueue)

```c
queue q = Queue.new(32);
Queue.enqueue(q, obj);
object front = Queue.dequeue(q);
```

### String utilities (IString)

All functions allocate with `Mem.alloc`. Caller must `String.free()` the result when done.

```c
string s1 = String.dupe("hello");
string s2 = String.concat(s1, " world");
string s3 = String.format("value = %d", 42);
int eq = String.compare(s1, s2);         // 0 if equal
String.free(s1);
String.free(s2);
String.free(s3);
```

### StringBuilder – efficient concatenation (IStringBuilder)

```c
string_builder sb = StringBuilder.new(256);
StringBuilder.append(sb, "hello");
StringBuilder.appendf(sb, " %s %d", "world", 2025);
StringBuilder.appendl(sb, NULL);         // just newline
string result = StringBuilder.toString(sb);  // allocates final string
printf("%s\n", result);

StringBuilder.free(sb);                  // frees builder + internal buffer
String.free(result);                     // free final string
```

### Iterator usage (IIterator)

Works on List and StringBuilder (and SlotArray in future).

```c
iterator it = List.iterateRange(my_list, 5, 20);   // optional range [5,20)
while (Iterator.hasNext(it)) {
    object o = Iterator.next(it);
    // use o
}
Iterator.free(it);
```

Predicate search:

```c
int is_target(object o) { return o == target; }

object found;
if (Iterator.findNext(it, is_target, &found))
    ; // found
```

### Notes

- All container contents are raw `object` pointers. No automatic cleanup.
- No exceptions, no longjmp. Errors are silent (NULL returns) or assert in debug.
- Thread-safety: none. Protect with your own mutexes.
- Tested on x86-64 and aarch64, Linux / Windows / macOS, clang/gcc/msvc.

That’s it. Include, link, use.
```

No badges, no emojis, no “getting started” fluff. Pure technical documentation.
```
