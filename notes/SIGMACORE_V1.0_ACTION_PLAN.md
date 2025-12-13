# Sigma-Core v1.0.0 – 7-Day Sprint Plan  
(Dec 7–13, 2025 | Pure C23, zero macros, modular OOP-in-C discipline enforced.)

As PM, I'm locking this plan. You're the dev—execute daily commits. We hit v1.0.0 on Dec 13 with modular headers, unified `sc_array` core, and Map/OrderedMap ready. No fluff: just achievable tasks, git branches, and deliverables. Timeline assumes 8–10 hr days; if blockers, we pivot to must-haves (modularization + array refactor).

### Key Principles (Locked)
- **Modularization**: Headers under `include/sigcore/`. Each self-contained with deps auto-included via nested `#include`. `sigcore.h` aggregates all (like .NET `using System;`). Users pick granular: `#include "sigcore/list.h"` pulls only list + deps (array.h, memory.h).
- **Array Core**: All collections build on `struct sc_array { addr* bucket; addr last; addr end; };`. Iterators point directly—no copies. This unifies List/Queue/SlotArray/Map under one pattern.
- **Map/OrderedMap**: Shared impl in `map.c`. `OrderedMap` reuses entry `next` for insertion-order list. One `sc_map_i` interface, two globals.
- **No Macro Hell**: Ever. Full stop.
- **Build**: CMake for libsigcore.a + tests. Sigma-Test integrated.
- **Release**: Git tag v1.0.0 on Dec 13. README updated with examples.

### 7-Day Outline
**Day 1: Dec 7 (Today) – Modularization Kickoff**  
- Branch: `refactor/modular-headers`  
- Split `sigcore.h` into `include/sigcore/{memory,array,list,queue,slot_array,string,string_builder,debug}.h`.  
- Add `include/sigcore/core.h` as aggregator (`#include` all sub-headers).  
- Update .c files to match (e.g., `list.c` includes `sigcore/list.h`).  
- Rename to `sc_` convention (e.g., `sc_list_new`, `sc_list_dispose`).  
- Deliverable: Buildable lib with modular includes. Commit + PR by EOD.  
- Time: 4–6 hrs. Test: Compile examples with granular includes.

**Day 2: Dec 8 – Array Core Refactor**  
- Branch: `feat/array-core` (from Day 1)  
- Introduce `sigcore/array.h`: `struct sc_array`, `sc_array_i` (count, clear, copy_to, etc.), globals `Array` / `Iter`.  
- Refactor List/Queue/SlotArray to embed `sc_array a;`. Update funcs (e.g., `sc_list_add` uses array internals).  
- Ensure iterators zero-copy: `sc_iter_new(list->a.bucket, list->a.last, ADDR_SIZE)`.  
- Deliverable: All collections use unified array. Passing Sigma-Test suite.  

**Day 3: Dec 9 – Memory Layer + Specializations**  
- Branch: `feat/memory`  
- Rename to `sc_memory_i Memory`. Add `realloc`.  
- Impl `Memory_arena_new(size_t)`: Bump allocator (fast, no free).  
- Impl `Memory_pool_new(size_t obj_size, int count)`: Fixed blocks.  
- Global switch: `Memory_set(&my_arena);` affects all allocs.  
- Update collections to use `Memory.realloc` for resizes.  
- Deliverable: Switchable allocators. Tests for arena/pool leaks.

**Day 4: Dec 10 – Map Implementation**  
- Branch: `feat/map`  
- New `sigcore/map.h`: `struct sc_map { sc_array entries; uint32_t size; /* hash state */ };`.  
- Impl robin-hood hashing in `map.c`: `sc_map_put`, `sc_map_get`, `sc_map_remove`.  
- Shared `sc_map_entry { addr key, val, next; }`.  
- Iterator: Hash-order walk.  
- Deliverable: Basic Map working. Sigma-Test for put/get/remove.

**Day 5: Dec 11 – OrderedMap + Unification**  
- Branch: `feat/ordered-map` (from Day 4)  
- New `sigcore/ordered_map.h`: Same struct + `sc_array order_links;`.  
- Wire `next` for doubly-linked insertion order.  
- `sc_iter` on OrderedMap walks links (zero-copy).  
- Reuse 90% of `map.c` code via flags/personality.  
- One `sc_map_i` interface for both globals: `Map` / `OrderedMap`.  
- Deliverable: Ordered iteration. Full tests.

**Day 6: Dec 12 – Deque + Polish + Tests**  
- Branch: `feat/deque`  
- New `sigcore/deque.h`: Double-ended queue via `sc_array` (circular). `sc_deque_push_front/back`, etc.  
- Fix String/StringBuilder to new naming/array.  
- Full Sigma-Test coverage: 80%+. Add leak checks.  
- CMake: libsigcore.a + minimal variants.  
- Deliverable: All features integrated. Green tests.

**Day 7: Dec 13 – Release v1.0.0**  
- Branch: `release/v1.0.0`  
- Update README: Tech specs, examples (modular includes, array usage, Map/OrderedMap).  
- Docs: Doxygen or inline comments.  
- Tag + push: `git tag v1.0.0`.  
- Deliverable: Published repo. Announce on X/Reddit.

### Objective Evaluation
Feasible: Yes, but aggressive for a lone dev. Strengths: Builds on existing code (e.g., collections.c → array.c). Unified array cuts redundancy. Shared Map impl saves time. Risks: Debug overruns on hashing/ordering—mitigate with daily tests. If Day 4/5 slip, drop Deque (nice-to-have). Overall: 80% confidence. We'll hit modularization + array core by Day 3 (must-haves), Maps by release (high-value). Pure C efficiency shines here—no bloat, zero macros. Full stop.

Commit today. Let's ship.
