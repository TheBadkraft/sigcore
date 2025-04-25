Here’s your refined documentation in the requested format, with added clarity and consistency while preserving your structure:

---

# **SigCore Library**  

## **Overview**  
**SigCore** is a lightweight C library offering foundational utilities for **memory management**, **dynamic collections**, and **string manipulation**. Designed for simplicity and performance, it provides a consistent interface for allocation, list operations, and efficient string building—ideal for embedded systems, game engines, and high-performance applications.  

### **Purpose**  
- **Memory Management**: Centralized allocation/deallocation with optional tracking.  
- **Dynamic Collections**:  
  - `List`: Growable arrays with automatic compaction.  
  - `SlotArray`: Sparse arrays with stable indices.  
  - `Queue`: Circular FIFO buffer.  
- **String Building**: Optimized concatenation with minimal allocations.  
- **Iterators**: Uniform traversal across collections.  

---

## **Installation**  
### **1. Clone the Repository**  
```bash  
git clone <repo-url>  
cd sigcore  
```  

### **2. Build the Library**  
```bash  
make  
```  

### **3. Install**  
```bash  
sudo make install  
```  
Installs `libsigcore.so` to `/usr/lib` and `sigcore.h` to `/usr/include`.  

---

## **Usage**  
### **Memory Management (`IMem`)**  
Handles allocation for all SigCore components:  
```c  
object ptr = Mem.alloc(1024);  // Allocate 1KB  
Mem.free(ptr);                // Free memory  
```  

**Advanced Features**:  
```c  
Mem_track(external_ptr);      // Track externally allocated memory  
Mem_freeWith(ptr, custom_free); // Custom deallocator  
```  

---

### **Dynamic Collections**  
#### **1. List (`IList`)**  
Growable array with O(1) access:  
```c  
list l = List.new(5);         // Create list (capacity=5)  
List.add(l, obj);             // Add object  
object item = List.getAt(l, 0); // Retrieve by index  
List.free(l);                 // Free list  
```  

#### **2. SlotArray (`ISlotArray`)**  
Sparse array with stable indices:  
```c  
slot_array sa = SlotArray.new(10);  
SlotArray.add(sa, obj);       // Insert at next empty slot  
object item;  
SlotArray.tryGetAt(sa, 3, &item); // Check slot 3  
SlotArray.free(sa);  
```  

#### **3. Queue (`IQueue`)**  
Circular FIFO buffer:  
```c  
queue q = Queue.new(8);  
Queue.enqueue(q, obj);        // Add to rear  
object front = Queue.dequeue(q); // Remove from front  
Queue.free(q);  
```  

---

### **Iterators (`IIterator`)**  
Uniform traversal for all collections:  
```c  
iterator it = Array.getIterator(list_obj, LIST);  
while (Iterator.hasNext(it)) {  
    object item = Iterator.next(it);  
    // Process item  
}  
Iterator.free(it);  
```  

**Filtered Iteration**:  
```c  
int is_non_null(object obj) { return obj != NULL; }  
Iterator.findNext(it, is_non_null, &item); // Skip empty slots  
```  

---

### **String Building (`IStringBuilder`)**  
Efficient concatenation:  
```c  
string_builder sb = StringBuilder.new(32);  
StringBuilder.append(sb, "Hello");  
StringBuilder.appendf(sb, ", %s!", "World");  
string result = StringBuilder.toString(sb); // "Hello, World!"  
StringBuilder.free(sb);  
```  

**Stream Output**:  
```c  
StringBuilder.toStream(sb, stdout); // Write to console  
```  

---

## **Why SigCore?**  
- **Zero Bloat**: No unnecessary features or dependencies.  
- **Stable Indices**: `SlotArray` preserves references for handle-based systems.  
- **Extensible**: Wrap primitives in domain-specific types (e.g., `EntityManager`).  
- **Thread-Unsafe by Design**: Opt-in safety via user-managed locks.  

**Upcoming Features**:  
- `HashMap` (open-addressing, `SlotArray`-backed).  
- Thread-safe variants (compile-time flags).  

--- 

**Need More?** See the full API in [`sigcore.h`](https://github.com/TheBadkraft/sigcore/blob/main/include/sigcore.h).

Building and Testing
Build: make -- creates bin/lib/libsigcore.so.
Test: make test_string_builder -- runs StringBuilder tests (requires sigtest).
Clean: make clean -- clears build files, keeps directories.

Contributing
Fork the repo, make changes, submit a pull request.
Use Doxygen comments in source/header files—see sigcore.h for style.

License
[GNU GENERAL PUBLIC LICENSE](/LICENSE) Version 3, 29 June 2007