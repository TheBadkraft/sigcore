**SigmaCore – Unified Master Backlog**  
**Locked as of Dec 12 2025 – v1.0.0 ships tomorrow, everything else lives here**  
Pure C23. Zero macros. Full stop.

| Milestone | Status   | ID        | Feature / Task                                                                 | Effort | Impact | Target Date | Notes / Acceptance Criteria |
|---------|----------|-----------|--------------------------------------------------------------------------------|--------|--------|-------------|-----------------------------|
| **v1.0.0** | TODO     | REC-001a  | Unified zero-copy iterator (`iterator.h` + `Iterator` interface)              | M      | 5 stars     | **Dec 13**  | Zero alloc, works on List / SlotArray / Collection, 5+ passing tests |
| **v1.0.0** | TODO     | REC-001b  | `String.h` interface - copy existing implementation                           | M      | 5 stars     | **Dec 13**  |  |
| **v1.0.0** | TODO     | REC-001c  | `StringBuilder` interface + copy existing implementation                      | M      | 5 stars     | **Dec 13**  |  |
| **v1.0.0** | TODO     | REC-005   | Full const-correctness on every public interface                              | S      | 4 stars     | **Dec 13**  | All read-only params → `const`, zero `-Wcast-qual` warnings |
| **v1.0.0** | TODO     | REC-009   | Complete README with copy-paste examples + badge                               | S      | 3 stars     | **Dec 13**  | List, SlotArray, Collections view/copy, new iterator usage |
| **v1.0.1** | Backlog  | REC-002   | Replace memcmp remove with comparator callback                                | S      | 4 stars     | Dec 20      | `collection_remove_if(pred, userdata)` + default memcmp |
| **v1.0.1** | Backlog  | REC-003   | Bulk operations for List (`append_range`, `insert_range`, `add_all`)          | S      | 4 stars     | Dec 20      | Unskip the two tests in test_list.log |
| **v1.0.1** | Backlog  | REC-006   | Allocation-failure injection tests                                             | S      | 3 stars     | Dec 20      | Hook Memory.alloc to fail after N calls → graceful NULL/-1 |
| **v1.0.2** | Backlog  | REC-004   | List shrink-to-fit + optional auto-compact                                     | M      | 3 stars     | Jan 2026    | `list_shrink`, optional threshold-based compact |
| **v1.0.2** | Backlog  | REC-007   | SlotArray reserve / preallocate                                                | S      | 2 stars     | Jan 2026    | `slotarray_reserve(sa, additional)` |
| **v1.1.0** | Backlog  | MEM-001   | **Arena allocator – bump + frame support**                                     | M      | 5 stars     | Feb 2026    | Full `sc_arena_i` as posted, current-arena workflow, `Memory.create_arena` |
| **v1.1.0** | Backlog  | MEM-002   | **Pool allocator – fixed-size object pool**                                    | M      | 5 stars     | Feb 2026    | Full `sc_pool_i` as posted, current-pool workflow, `Memory.create_pool` |
| **v1.1.0** | Backlog  | MEM-003   | **Memory introspection & leak dump**                                           | S      | 4 stars     | Feb 2026    | `Memory.tracked_count`, `Memory.page_count`, `Memory.dump_leaks()` |
| **v1.1.0** | Backlog  | MEM-004   | **Memory becomes factory for heap + arena + pool**                             | S      | 4 stars     | Feb 2026    | Final public API exactly as shown in your example |
| **v1.1.0** | Backlog  | REC-008   | Performance benchmark suite (1k–1M elements)                                   | L      | 3 stars     | Feb 2026    | Compare List vs SlotArray vs raw C array, publish results |
| **v1.1.0** | Backlog  | REC-010   | Optional destructor callback for collections                                   | M      | 2 stars     | Feb 2026    | `collection_set_dtor(coll, void(*dtor)(object))` |

NOTE: last 3 features for **SigmaCore** to release are `String`, `StringBuilder`, and `Iterator` ... 2 of those, the _string_ APIs will be unified under a `sigmacore/strings.h`. These are essentially done and will just be copied over with the tests ... iterator gets a slight upgrade but real implementation is mostly a copy over and will belong to the unified `sigmacore/collections.h` header and the `Collections` interface. Not much in the way of real refactoring left with these ... they will mostly work as is and anything _not working_ will be quickly identified and fixed by COB release tomorrow, the 13th.

### Full Scope of Memory refactoring and upgrade (Dec 12 2025)
**SigmaCore Memory Management System Capabilities**

#### Core API
- **`Memory.alloc(size, zero_init)`** - Allocates memory blocks with optional zero-initialization
- **`Memory.dispose(ptr)`** - Frees tracked memory blocks  
- **`Memory.realloc(ptr, new_size)`** - Resizes existing allocations
- **`Memory.is_tracking(ptr)`** - Validates if a pointer is currently tracked
- **`Memory.track(ptr)` / `Memory.untrack(ptr)`** - Manual tracking control

#### Growth Mechanics

**Page-Based Architecture:**
- Memory tracking uses a **linked list of pages**, each containing a SlotArray tracker
- **Initial page** uses static buffer (4096 slots) to avoid bootstrap allocation issues
- **Dynamic pages** created on-demand with growable SlotArrays (up to 16 total pages)
- **Page expansion** triggered when current page's SlotArray cannot accommodate new allocations

**SlotArray Growth:**
- SlotArrays **reuse empty slots** before growing (hole filling)
- **Growth factor**: Doubles capacity when full (if `can_grow = true`)
- **Static protection**: Bootstrap SlotArray (`can_grow = false`) prevents self-tracking loops
- **Dynamic allocation**: New pages use raw `malloc/free` for internal buffers to avoid recursion

#### Index Location Across Pages

**Cross-Page Traversal:**
- **Linear search** across all pages for `is_tracking()` and `dispose()` operations
- **Page linking**: Doubly-linked pages (`next`/`prev`) enable bidirectional traversal
- **Page registry**: PArray maintains ordered list of all active pages
- **Current page pointer**: Tracks the active page for new allocations

**Index Resolution:**
- Each allocation gets a **local index** within its page's SlotArray
- **Global lookup** requires scanning all pages (O(n) complexity)
- **No global index mapping** - pointers are located by value comparison across pages

#### Cross-Page Operations

**Allocation Strategy:**
1. Attempt allocation in current page
2. If current page full → create new page and retry
3. Link new page to chain and update registry
4. Switch current page to new page

**Disposal Process:**
1. **Traverse all pages** from first to last
2. **Scan each page's SlotArray** for matching pointer
3. **Remove from tracker** when found (early exit)
4. **Free underlying memory** regardless of tracking status

**Tracking Validation:**
- **Cross-page search** ensures pointers are found regardless of creation page
- **Page order preservation** maintains allocation chronology
- **Hole filling** prioritizes reuse in existing pages before expansion

#### Filling and Hole Management

**Hole Filling Priority:**
- **Empty slot reuse** happens before any growth attempt
- **Intra-page filling** - allocations fill holes in current page first
- **Page-level optimization** - prevents unnecessary page creation
- **Tested behavior**: 50 new allocations reuse holes instead of creating new page

**Memory Efficiency:**
- **No external fragmentation** - SlotArrays manage logical tracking separately from physical memory
- **Compact representation** - addr-sized slots track arbitrary-sized allocations
- **Zero overhead** for untracked allocations (direct malloc/free)

#### Safety & Validation

**Self-Tracking Prevention:**
- **Static bootstrap buffer** avoids initial allocation tracking
- **Raw malloc/free** for internal page/SlotArray structures  
- **Growth restrictions** on static buffers prevent disposal loops

**Cross-Page Validation:**
- **Comprehensive testing**: Multi-page creation, cross-page tracking, hole filling, stress tests
- **Leak detection**: All tracked pointers verified across page boundaries
- **Boundary conditions**: Handles page limits (16 max), allocation failures, teardown cleanup

The system provides robust, leak-free memory management with automatic multi-page scaling while maintaining O(1) allocation and O(n) tracking/lookup performance characteristics.

### Current State Summary (Dec 12 2025)
- **Blockers to v1.0.0 tag**: exactly 3 items (REC-001, REC-005, REC-009) → **must be closed tomorrow**
- **Everything else**: officially locked into this backlog
- **v1.0.0 ships**: Dec 13 with current collections (List, SlotArray, FArray, PArray, Collections) + iterator + const-correct + beautiful README
- **v1.1.0**: Memory revolution (Arena + Pool + current-context workflow) – already 100 % designed and copy-paste ready

### **SigmaCore v1.x+ Memory Arena/Pool Overview

#### 1. Arena — Bump allocator with frame support (v1.x)

```c
// include/sigcore/arena.h
typedef struct sc_arena* arena;

typedef struct sc_arena_i {
    arena  (*create)(usize total_bytes);
    void   (*destroy)(arena a);

    void   (*reset)(arena a);
    object (*alloc)(arena a, usize size);
    object (*zalloc)(arena a, usize size);
    void   (*begin_frame)(arena a);
    void   (*end_frame)(arena a);
    usize  (*used)(arena a);
    bool   (*contains)(arena a, const void* ptr);

    // current-arena workflow (99 % of code uses this)
    void   (*set_current)(arena a);
    void   (*reset_current)(void);
    object (*alloc_current)(usize size);
    object (*zalloc_current)(usize size);
    void   (*begin_frame_current)(void);
    void   (*end_frame_current)(void);
    usize  (*used_current)(void);
    arena  (*get_current)(void);
} sc_arena_i;

extern const sc_arena_i Arena;
```

**Memory gains these two functions**

```c
// include/sigcore/memory.h  (added in v1.x)
arena (*create_arena)(usize total_bytes);
void  (*destroy_arena)(arena a);
```

#### 2. Pool — Fixed-size object pool (v1.x)

```c
// include/sigcore/pool.h
typedef struct sc_pool* pool;

typedef struct sc_pool_i {
    pool   (*create)(usize object_size, usize initial_capacity);
    void   (*destroy)(pool p);

    object (*alloc)(pool p);                    // always zeroed
    void   (*free)(pool p, object obj);
    void   (*reset)(pool p);                    // all objects become free again
    usize  (*count_used)(pool p);
    usize  (*count_free)(pool p);
    usize  (*capacity)(pool p);
    bool   (*contains)(pool p, const void* ptr);

    // current-pool workflow
    void   (*set_current)(pool p);
    object (*alloc_current)(void);
    void   (*free_current)(object obj);
    void   (*reset_current)(void);
    pool   (*get_current)(void);
} sc_pool_i;

extern const sc_pool_i Pool;
```

**Memory gains these two functions**

```c
// include/sigcore/memory.h  (added in v1.x)
pool (*create_pool)(usize object_size, usize initial_capacity);
void (*destroy_pool)(pool p);
```

#### 3. Memory itself gains a few tiny introspection helpers (v1.x)

```c
// include/sigcore/memory.h  (added in v1.x)
typedef struct sc_memory_i {
    // existing v1.0 functions ...
    object (*alloc)(usize);
    object (*zalloc)(usize);
    object (*realloc)(object, usize);
    void   (*free)(object);
    bool   (*has)(object);
    bool   (*track)(object);

    // v1.x+ arena & pool factories
    arena  (*create_arena)(usize total_bytes);
    void   (*destroy_arena)(arena a);
    pool   (*create_pool)(usize object_size, usize initial_capacity);
    void   (*destroy_pool)(pool p);

    // new introspection (optional but extremely useful)
    usize  (*tracked_count)(void);      // total live tracked pointers
    usize  (*page_count)(void);         // how many tracker pages exist
    void   (*dump_leaks)(void);         // prints every tracked pointer still alive
} sc_memory_i;

extern const sc_memory_i Memory;
```

### Final Public Memory API in v1.x+

```c
// Example usage — this is what your game code will actually look like
arena   frame_arena  = Memory.create_arena(128 * 1024);
pool    bullet_pool  = Memory.create_pool(sizeof(Bullet), 2048);

Arena.set_current(frame_arena);
Pool.set_current(bullet_pool);

void update_game(void)
{
    Arena.reset_current();

    Bullet* b = Pool.alloc_current();           // zeroed, O(1)
    b->pos = player_pos;

    Arena.begin_frame_current();
    TempData* temp = Arena.zalloc_current(sizeof(TempData));
    // ... do risky work
    if (failed) {
        Arena.end_frame_current();              // discard everything
    } else {
        Arena.end_frame_current();              // keep everything
    }
}

// At shutdown
Memory.destroy_arena(frame_arena);
Memory.destroy_pool(bullet_pool);
```

### Summary — What v1.x adds on top of v1.0

| Feature                  | v1.0 (Friday)       | v1.x (next ???)        |
|--------------------------|---------------------|------------------------|
| Global heap + tracking   | Yes (multi-page)    | Yes                    |
| Arena (bump + frames)    | No (backlog)        | Yes                    |
| Pool (fixed-size)        | No (backlog)         | Yes                    |
| Current-context workflow | No                  | Yes (both Arena & Pool)|
| Memory as factory        | Only heap           | Heap + Arena + Pool    |
| Leak dump / debug        | No                  | Yes (`Memory.dump_leaks`) |

