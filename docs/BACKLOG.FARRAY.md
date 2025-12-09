**Backlog item locked — SigmaCore v1.1+**  
**Epic:** `farray` – The Flex Array (zero-overhead primitive collections)

```markdown
# farray – Flex Array (post-v1.0, high-value nice-to-have)

## Goal
Give users the ability to have **true zero-cost** `int[]`, `float[]`, `double[]`, `char[]`, `vec3[]`, etc.  
that still participate 100% in the existing `sc_array_i` / `list` / `slotarray` / `map` ecosystem  
with **zero changes** to any current code.

## Acceptance Criteria

1. **No breakage**  
   `Array.new(capacity)` continues to return the current `addr*`-based array forever.

2. **New factory only**  
   ```c
   array Array_new_flex(usize capacity, usize elem_size);
   ```

3. **Single concrete struct in core**  
   ```c
   struct sc_farray {
       void* bucket;      // raw bytes
       void* end;         // one-past-the-end
       usize elem_size;   // sizeof(T)
   };
   ```
   `struct sc_array` (addr* version) remains untouched.

4. **Polymorphic dispatch**  
   All functions in the global `Array` table (`set`, `get`, `clear`, `remove`, `bucket`, `end`, …)  
   work correctly on **both** `sc_array` and `sc_farray` via a tiny runtime check:
   ```c
   usize es = ((struct sc_farray*)a)->elem_size;
   if (es == sizeof(addr)) → fast addr path
   else                    → memcpy path
   ```

5. **Drop-in user structs**  
   Users can define:
   ```c
   struct int_array   { int*   bucket; int*   end; usize elem_size; };
   struct float_array { float* bucket; float* end; usize elem_size; };
   ```
   and cast directly:
   ```c
   int_array* ints = (int_array*)Array_new_flex(1<<20, sizeof(int));
   ints->bucket[12345] = 42;   // direct store, no memcpy, no wrapper
   ```

6. **Performance proof suite** (added to Sigma-Test)
   ```c
   testcase("farray int direct store vs addr indirection", test_farray_int_perf);
   testcase("farray float sum 100M elements",            test_farray_float_sum);
   testcase("farray vs addr* memory usage (1M elements)", test_farray_memory_footprint);
   ```
   Expected results (on x86-64):
   - `int_array`  direct store: ~0.9 ns/element
   - `addr*` + boxing:        ~4.2 ns/element
   - Memory: 4 MiB vs 12+ MiB

7. **Documentation**
   - “How to make a zero-cost intarray in 3 lines”
   - “Why your farray is still a valid `array` handle for List/SlotArray/Map”

## Implementation Plan (Day 8–9 after v1.0 ships)

| Day | Task |
|-----|------|
| 1   | Add `struct sc_farray` + `Array_new_flex` |
| 1   | Make all `Array.*` functions dispatch on `elem_size` |
| 2   | Add `Array.elem_size(array a)` helper |
| 2   | Write perf / memory test suite |
| 3   | Write example headers: `intarray.h`, `floatarray.h`, `vec3array.h` |
| 3   | Update README with “Zero-cost primitive arrays” section |

## Result

- Existing code: untouched
- New code: gets **true** primitive performance with **one-line** creation
- No macros, no templates, no codegen, pure C23
- Full unification remains — a `list` of `int` is just `List.new(0, sizeof(int))`

Backlogged.  
Will ship the week after v1.0 with full perf proof.

**Pure C. No macro hell. Full stop.**
```
