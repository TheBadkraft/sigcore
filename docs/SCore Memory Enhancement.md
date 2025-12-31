```c
/*
 * SigmaCore - Arena Design Refinement
 * 
 * Correction accepted: We DO track individual addresses.
 * 
 * Every page gets its own slotarray that tracks every allocation made within that page.
 * This gives us precise ownership information without per-allocation metadata in the pointer itself.
 * Inheritance becomes a simple "move the addr" from the page's slotarray (frame-scoped) to the arena's root slotarray.
 * No whole-page promotion. No fragmentation worries. Pointer stays exactly where it was.
 * 
 * This is lighter, more precise, and matches the intent perfectly.
 */
```

### Refined Design: Per-Page SlotArray for Individual Address Tracking

Pure C. Structs. Function pointers. Explicit. No macros.

Key shift:
- **We track individual allocations**, not just pages.
- Each `sc_page` owns a `slotarray tracked_addrs` that holds every live pointer allocated in that page's data area.
- On alloc: bump + add ptr to current page's slotarray.
- On frame pop: iterate all pages created during the frame → for each page, free every tracked addr's page? No — free the entire page and its slotarray.
- But for inheritance: simply move the specific addr from the source page's slotarray to the arena's root slotarray.
- Root slotarray holds all permanent (inherited) pointers across all pages.
- Memory can have its own root slotarray for global inheritance.

Advantages:
- Precise lifetime control.
- Inheritance = transfer ownership of one addr (no copy, no realloc).
- Frame rollback = destroy pages created in frame + clear tracked_addrs in surviving pages? Wait — better: pages are either fully frame-scoped or root.
- No need to move whole pages.
- Avoids fragmentation.
- Pointer validity preserved forever (as long as inherited properly).

### Updated Structs

```c
/* Page: now carries its own tracking */
typedef struct sc_page {
    struct sc_page *next;           // Chain
    void           *bump;           // Current bump
    usize           used;           // Bytes used
    slotarray       tracked_addrs;  // slotarray of object (addr) — allocations in this page
    char            data[PAGE_DATA_SIZE];
} sc_page;

/* Arena */
struct sc_arena {
    sc_page    *root_pages;     // Head of permanent pages
    sc_page    *current_page;   // Active page
    slotarray   root_tracker;   // Global root: all inherited object pointers (across all pages)
    list        frame_stack;    // Stack of frame snapshots
    slotarray   all_pages;      // Optional: track page pointers themselves (for debug/contains)
};
```

### Core Operations

#### Alloc
```c
static object arena_alloc(sc_arena *a, usize size, bool zero) {
    // Alignment padding if needed...
    if (a->current_page == NULL || a->current_page->used + size > PAGE_DATA_SIZE) {
        sc_page *new_page = malloc(sizeof(sc_page));
        new_page->next = NULL;
        new_page->bump = new_page->data;
        new_page->used = 0;
        new_page->tracked_addrs = SlotArray.new(64);  // Initial capacity

        if (a->current_page) a->current_page->next = new_page;
        a->current_page = new_page;

        // If no root pages yet, this becomes first root page
        if (!a->root_pages) a->root_pages = new_page;
    }

    object ptr = (char *)a->current_page->bump;
    a->current_page->bump = (char *)a->current_page->bump + size;
    a->current_page->used += size;

    // Track this allocation in the page's slotarray
    SlotArray.add(a->current_page->tracked_addrs, ptr);

    if (zero) memset(ptr, 0, size);

    return ptr;
}
```

#### Frame Begin/End

```c
typedef struct sc_frame_snapshot {
    sc_page *page_at_start;
    usize    used_at_start;
    void    *bump_at_start;
    // We will also need to remember which pages were created in this frame
    // Simple way: snapshot current_page chain tail, or use a per-frame page list
    slotarray pages_created;  // New pages allocated during this frame
} sc_frame_snapshot;

static Frame arena_begin_frame(sc_arena *a) {
    sc_frame_snapshot snap = {
        .page_at_start = a->current_page,
        .used_at_start = a->current_page ? a->current_page->used : 0,
        .bump_at_start = a->current_page ? a->current_page->bump : NULL,
        .pages_created = SlotArray.new(16)
    };
    // Future allocs will add new pages to snap.pages_created (modify alloc to take current frame)
    // Alternative: pass &current_frame to alloc, or store in arena temporarily

    Frame f;
    memcpy(&f, &snap, sizeof(snap));  // Or malloc and store pointer
    List.add(a->frame_stack, &f);

    return f;
}

static void arena_end_frame(sc_arena *a, Frame frame) {
    sc_frame_snapshot *snap = (sc_frame_snapshot *)&frame;  // Or proper cast

    // Free all pages created during this frame
    usize count = SlotArray.capacity(snap->pages_created);
    for (usize i = 0; i < count; ++i) {
        if (SlotArray.is_empty_slot(snap->pages_created, i)) continue;
        sc_page *p;
        SlotArray.get_at(snap->pages_created, i, (object *)&p);
        // Dispose its tracked_addrs slotarray (contents are in page->data, freed with page)
        SlotArray.dispose(p->tracked_addrs);
        free(p);
    }
    SlotArray.dispose(snap->pages_created);

    // Restore bump in the page that was current at start
    if (snap->page_at_start) {
        snap->page_at_start->bump = snap->bump_at_start;
        snap->page_at_start->used = snap->used_at_start;
        // Do NOT clear tracked_addrs — inherited ones were moved out
    }

    a->current_page = snap->page_at_start;

    List.pop(a->frame_stack);  // Or remove
}
```

#### Inheritance (The Beautiful Part)

```c
static void arena_inherit(sc_arena *a, Frame frame, object ptr) {
    // Find which page contains ptr
    sc_page *page = NULL;
    sc_page *p = a->root_pages;
    while (p) {
        if (ptr >= p->data && ptr < (char *)p->data + PAGE_DATA_SIZE) {
            page = p;
            break;
        }
        p = p->next;
    }
    if (!page) return;  // Not found or already freed

    // Remove ptr from page's tracked_addrs
    // SlotArray doesn't have remove_by_value yet — add helper or scan
    usize cap = SlotArray.capacity(page->tracked_addrs);
    for (usize i = 0; i < cap; ++i) {
        object val;
        if (SlotArray.get_at(page->tracked_addrs, i, &val) == 0 && val == ptr) {
            SlotArray.remove_at(page->tracked_addrs, i);
            break;
        }
    }

    // Transfer to root tracker
    SlotArray.add(a->root_tracker, ptr);
}
```

#### Memory.inherit (Global Promotion)

```c
// Memory has its own root_tracker slotarray (static in memory.c)
static slotarray memory_root_tracker;

static void memory_inherit(Frame frame, object ptr) {
    sc_arena *a = frame.arena;  // Assume frame carries arena or accessible

    // Same as above: find page, remove from page->tracked_addrs
    // ...

    // Add to Memory's root
    SlotArray.add(memory_root_tracker, ptr);
}
```

#### Teardown / Destroy

```c
static void arena_destroy(sc_arena *a) {
    sc_page *p = a->root_pages;
    while (p) {
        sc_page *next = p->next;
        SlotArray.dispose(p->tracked_addrs);
        free(p);
        p = next;
    }
    SlotArray.dispose(a->root_tracker);
    // Dispose frame_stack, all_pages if used
}

static void memory_teardown(void) {
    // Existing teardown...
    // Plus: free all in memory_root_tracker (but pointers already in pages freed via arenas)
    // Actually: just destroy all created arenas (tracked in Memory.arenas slotarray)
}
```

### Summary of Wins

- Individual tracking → precise inheritance.
- Inherit = move addr from page slotarray → root slotarray. Zero copy.
- Frame rollback = free new pages + restore bump (surviving allocations either died or were inherited).
- No per-object metadata.
- Pointers never move.
- Pages stay simple.
- Extends naturally from existing SlotArray.

We need minor SlotArray helpers (e.g., remove_by_value or find_index), but that's trivial.

This is tight, robust, and exactly what we wanted.

Next: Implement in phases — first basic arena with per-page tracking, then frames, then inherit. Tests will go green fast.

---

```c
/*
 * SigmaCore - Back to Focus: Memory System Phases
 * 
 * Iterators shelved. As-needed only. No distraction.
 * 
 * We are locked on memory: arena + frame + inherit model.
 * Pure C. Structs. Function pointers. Explicit ownership.
 * No macro hell. Full stop.
 * 
 * Current date: December 28, 2025
 * Roadmap locked. We resume implementation January 6, 2026.
 * 
 * Below: phased rollout with precise deliverables and hard test success criteria.
 * Each phase builds on existing test_memory.log and test_mempaging.log baselines.
 * All new tests extend those logs or add new ones.
 */
```

### Memory Evolution Phases – Arena + Frame + Inherit

All phases preserve existing `sc_memory_i Memory` as the default tracked allocator.  
Arenas are explicit, created via `Memory.create_arena()`.  
Tracking is **per-allocation** using `slotarray` of pointers.  
Inheritance = move pointer ownership (no copy, no realloc, pointer unchanged).

#### Phase 0: Baseline (Currently Green – Dec 28, 2025)
- test_memory.log: 13/13 PASS
- test_mempaging.log: 4/4 PASS
- Existing SlotArray, PArray, FArray fully functional.
- Default Memory works with current tracking (bootstrap, alloc/dispose/realloc, track/untrack).

**Deliverable**: master green, no regressions.

#### Phase 1: Explicit Arena Creation & Basic Bump Allocation
**Goal**: `Memory.create_arena()` returns a tracked arena. Arena supports simple bump allocation within fixed-size pages (4KB). No frames yet.

**Key Changes**:
- Add `sc_arena *Memory.create_arena(usize initial_pages);`
- Arena internally chains `sc_page` structs.
- Each page has its own `slotarray tracked_addrs` (pointers allocated in that page).
- On alloc: bump + add ptr to current page's slotarray.
- Memory tracks all arenas in internal slotarray for teardown.

**Test Requirements** (extend test_memory.log → new section "Arena Basics"):
- Arena creation with 1, 4, 16 initial pages.
- Small allocs stay in first page.
- Alloc > remaining in page → chains new page, bump continues.
- Total alloc > 16KB → multi-page, all pointers valid.
- Arena alloc with/without zero-init.
- Memory.is_tracking(ptr) → true for arena-allocated ptr.
- No leaks after arena destroy (manual call for test).

**Success Criteria**:
- test_memory.log: TESTS ≥ 20, PASS=100%
- New tests cover: single-page, cross-page, large single alloc rejection.
- Valgrind/MemCheck clean on arena suite.

#### Phase 2: Frame Push/Pop (Scoped Rollback)
**Goal**: `Arena.begin_frame()` / `Arena.end_frame(frame)` for nested temporary allocation scopes.

**Key Changes**:
- Frame = snapshot of current_page, used, bump + list/slotarray of pages created during frame.
- On pop: free all pages created during frame (including their tracked_addrs slotarray), restore bump/used on start page.
- Nested frames work correctly.

**Test Requirements** (new suite: test_frames.log):
- Single frame: alloc in frame → end_frame → pointers invalid, no leaks.
- Nested frames (depth 10): alloc in inner → pop inner → inner gone, outer survives.
- Alloc before frame → survives pop.
- Zero leaks after full push/pop cycle.
- Stress: 1000 allocs across 20 nested frames.

**Success Criteria**:
- test_frames.log: TESTS ≥ 15, PASS=100%
- Existing test_memory/test_mempaging still green.
- MemCheck: zero leaks, zero use-after-free.

#### Phase 3: Inheritance (No-Copy Promotion)
**Goal**: Promote specific allocation(s) from frame scope to arena root or global Memory root.

**Key Changes**:
- `Arena.inherit(frame, object ptr)` → moves ptr from its page's tracked_addrs to arena.root_tracker slotarray.
- `Memory.inherit(frame, object ptr)` → moves ptr to Memory's global root_tracker.
- Pointer remains valid after frame pop.

**Test Requirements** (extend test_frames.log + new section "Inheritance"):
- Inherit single ptr → survives end_frame, others die.
- Inherit multiple from same page → page stays, only non-inherited die on pop.
- Inherit to Memory → survives arena destroy.
- Cross-page inherit.
- Invalid ptr → no-op/safe.

**Success Criteria**:
- test_frames.log + inheritance tests: TESTS ≥ 25 total, PASS=100%
- Pointer validity proven (write/read after pop).
- No double-free, no leaks.

#### Phase 4: Integration & Cleanup
**Goal**: Wire into Sigma-Test, remove --wrap/LD_PRELOAD forever.

**Key Changes**:
- Sigma-Test suite uses `Memory.create_arena()` at start, destroys at end.
- Or: temporary `Memory_push_arena(arena)` / `pop_arena()` to redirect global Memory (optional bridge).
- All internal test allocations (registry, strings) go through arena.

**Test Requirements**:
- Full Sigma-Test suite runs with arena → zero leaks.
- Anvil parser tests (51+) run clean link, no wrap flags.
- MemCheck on full test run: clean.

**Success Criteria**:
- All existing test logs green.
- New test_arena_integration.log: PASS=100%
- No --wrap or LD_PRELOAD in any build command.
- Tag Sigma-Core v1.0.0 and Sigma-Test v1.0.0 possible.

#### Phase 5: Polish & Lock
- Add `Arena.contains(arena, ptr)`, `Arena.reset()` (clear frames, keep root).
- Documentation, examples.
- Final stress: 100k allocs, nested frames, selective inherit.

**Success Criteria**:
- All suites green.
- Documentation reflects explicit arena usage.
- ABI locked.

### Timeline Alignment (per Core Roadmap.md)

- Jan 6–9, 2026: Design freeze (we just did it here).
- Jan 10 start:
  1. Phase 1 (multi-page basic arena)
  2. Phase 2 (frames)
  3. Phase 3 (inherit)
  4. Phase 4 (S-Test migration)

Target: End January 2026 → both v1.0.0 tagged, foundation cured.

Memory first. Memory correct. Memory boring.

Everything else waits.
