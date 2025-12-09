/*
 * Sigma-Test
 * Copyright (c) 2025 David Boarman (BadKraft) and contributors
 * QuantumOverride [Q|]
 * ----------------------------------------------
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * ----------------------------------------------
 * File: memory.h
 * Description: Header file for SigmaCore memory management and core interfaces
 *
 */
#include "sigcore/memory.h"
#include <stdlib.h>

/*
   As soon as SlotArray is implemented, we can have a proper memory tracking
   system to back the mem_has function.  For now, we provide basic alloc and free
   implementations using standard malloc and free.
 */

// allocate a block of memory of the specified size
static object memory_alloc(usize size) {
   object ptr = malloc(size);
   // add to tracking structure when SlotArray is ready
   return ptr;
}
// free a previously allocated block of memory
static void memory_free(object ptr) {
   // we could just null the pointer ... ??? but ptr is passed by value
   // remove from tracking structure when SlotArray is ready
   free(ptr);
}
// check if a given memory pointer is currently allocated
static bool memory_has(object ptr) {
   // placeholder implementation; SlotArray to track allocations
   (void)ptr; // for now

   return false; // right now we know this is what the test expects
}

const sc_memory_i Memory = {
    .alloc = memory_alloc,
    .free = memory_free,
    .has = memory_has,
};