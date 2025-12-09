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
 * File: list.c
 * Description: Source file for SigmaCore list definitions and interfaces
 *
 * List:    An ordered collection structure derived from Array that allows
 *          dynamic resizing, element insertion, removal, and retrieval by index,
 *          or by appending to the end.
 */
#include "sigcore/list.h"
#include "sigcore/collections.h"
#include "sigcore/internal/collections.h"
#include "sigcore/memory.h"

//  declare the List struct: derived from Array
struct sc_list {
   array bucket; // base array structure
   addr last;    // pointer to the last valid element (inclusive)
};

//  create new list with specified initial capacity
static list list_new(usize capacity) {
   //  allocate memory for the list structure
   struct sc_list *lst = Memory.alloc(sizeof(struct sc_list));
   if (!lst) {
      return NULL; // allocation failed
   }

   //  create the underlying array
   lst->bucket = Array.new(capacity);
   if (!lst->bucket) {
      Memory.free(lst);
      return NULL; // allocation failed
   }

   //  initialize last pointer to one before start (empty list)
   lst->last = array_get_bucket_start(lst->bucket) - ADDR_SIZE;

   return lst;
}
//  dispose of the list and free resources
static void list_dispose(list lst) {
   if (!lst) {
      return; // nothing to dispose
   }

   //  dispose of the underlying array
   Array.dispose(lst->bucket);

   //  free the list structure itself
   Memory.free(lst);
}
//  grow the list's underlying array to accommodate more elements
static int list_grow(list lst) {
   if (!lst) {
      return -1; // invalid list
   }

   // calculate new capacity (1.5x growth, minimum +1)
   usize current_capacity = Array.capacity(lst->bucket);
   usize new_capacity = (current_capacity * 3) / 2;
   if (new_capacity == current_capacity) {
      new_capacity++; // ensure growth
   }

   // create new array with larger capacity
   array new_bucket = Array.new(new_capacity);
   if (!new_bucket) {
      return -1; // allocation failed
   }

   // copy existing data
   addr old_start = array_get_bucket_start(lst->bucket);
   addr new_start = array_get_bucket_start(new_bucket);
   addr old_last = lst->last;

   for (addr pos = old_start; pos <= old_last; pos += ADDR_SIZE) {
      addr offset = pos - old_start;
      addr new_pos = new_start + offset;
      *((object *)new_pos) = *((object *)pos);
   }

   // update list pointers
   addr old_bucket_start = array_get_bucket_start(lst->bucket);
   addr new_bucket_start = array_get_bucket_start(new_bucket);
   lst->last = new_bucket_start + (lst->last - old_bucket_start);

   // dispose old array and update bucket
   Array.dispose(lst->bucket);
   lst->bucket = new_bucket;

   return 0; // success
}
//  get the current capacity of the list
static usize list_capacity(list lst) {
   if (!lst) {
      return 0; // invalid list
   }
   return Array.capacity(lst->bucket);
}
//  get the current size of the list
static usize list_size(list lst) {
   if (!lst) {
      return 0; // invalid list
   }
   addr start = array_get_bucket_start(lst->bucket);
   if (lst->last < start) {
      return 0; // empty list
   }
   return (int)(((lst->last - start) / sizeof(addr)) + 1);
}
//  append a value to the end of the list
static int list_append(list lst, object value) {
   if (!lst || !value) {
      return -1; // invalid parameters
   }
   //  get next position (one past current last)
   addr next_pos = lst->last + ADDR_SIZE;
   //  make sure we are within capacity
   addr bucket_end = array_get_bucket_end(lst->bucket);
   if (next_pos >= bucket_end) {
      // try to grow the list
      if (list_grow(lst) != 0) {
         return -1; // growth failed
      }
      // recalculate after growth
      bucket_end = array_get_bucket_end(lst->bucket);
      next_pos = lst->last + ADDR_SIZE;
   }

   //  append the value
   *((object *)next_pos) = value;
   lst->last = next_pos;

   return 0; // success
}
//  get the value at the specified index in the list
static int list_get_at(list lst, usize index, object *out_value) {
   // out_value is going to be NULL coming in
   if (!lst) {
      return -1; // invalid list
   }
   // validate the index
   addr start = array_get_bucket_start(lst->bucket);
   //  indices are incremented by ADDR size
   addr target_pos = start + (index * ADDR_SIZE);
   if (target_pos > lst->last) {
      return -1; // index out of bounds
   }
   // now use the array's get function
   return Array.get(lst->bucket, index, (addr *)out_value);
}
//  remove the element at the specified index from the list
static int list_remove_at(list lst, usize index) {
   int result = -1;
   if (!lst) {
      return result; // invalid list
   }
   // validate the index
   addr start = array_get_bucket_start(lst->bucket);
   addr target_pos = start + (index * ADDR_SIZE);
   if (target_pos > lst->last) {
      return result; // index out of bounds
   }
   // remove at index using the array's remove function
   result = Array.remove(lst->bucket, index);
   // do we need to condense array and reset last ... ???
   if (result == 0) {
      // compact the array to remove the hole
      usize non_empty_count = Collections.compact(lst->bucket);
      // reset last pointer based on non-empty count
      addr bucket_start = array_get_bucket_start(lst->bucket);
      if (non_empty_count == 0) {
         lst->last = bucket_start - ADDR_SIZE; // empty list
      } else {
         lst->last = bucket_start + ((non_empty_count - 1) * ADDR_SIZE);
      }
   }

   return result;
}
// set the value at the specified index in the list
static int list_set_at(list lst, usize index, object value) {
   // any reason we should prevent NULLs from being set?
   if (!lst) {
      return -1; // invalid parameters
   }
   // validate the index
   addr start = array_get_bucket_start(lst->bucket);
   addr target_pos = start + (index * ADDR_SIZE);
   // last is inclusive .. it is the last valid position, period
   if (target_pos > lst->last) {
      return -1; // index out of bounds
   }
   // set is an overwrite, so no need to adjust last pointer
   // now use the array's set function
   return Array.set(lst->bucket, index, (addr)value);
}
static int list_insert_at(list lst, usize index, object value) {
   // NULLs are allowed ... not my call, that's on the user
   if (!lst) {
      return -1; // invalid parameters
   }
   // validate the index
   addr start = array_get_bucket_start(lst->bucket);
   addr target_pos = start + (index * ADDR_SIZE);
   // allow insert at size (append)
   addr one_past_last = lst->last + ADDR_SIZE;
   if (target_pos > one_past_last) {
      return -1; // index out of bounds
   }
   // make sure we have capacity
   addr bucket_end = array_get_bucket_end(lst->bucket);
   if (one_past_last >= bucket_end) {
      // try to grow the list
      if (list_grow(lst) != 0) {
         return -1; // growth failed
      }
      // recalculate after growth
      bucket_end = array_get_bucket_end(lst->bucket);
      one_past_last = lst->last + ADDR_SIZE;
   }
   // shift elements right from index to last
   for (addr pos = one_past_last; pos > target_pos; pos -= ADDR_SIZE) {
      addr prev_pos = pos - ADDR_SIZE;
      *((object *)pos) = *((object *)prev_pos);
   }
   // insert the new value
   *((object *)target_pos) = value;
   // update last pointer
   lst->last += ADDR_SIZE;

   return 0; // success
}
// prepend a value to the start of the list
static int list_prepend(list lst, object value) {
   if (!lst || !value) {
      return -1; // invalid parameters
   }
   return list_insert_at(lst, 0, value);
}
// clear the contents of the list
static void list_clear(list lst) {
   if (!lst) {
      return; // invalid list
   }
   // clear the underlying array
   Array.clear(lst->bucket);
   // reset last pointer to one before start (empty list)
   lst->last = array_get_bucket_start(lst->bucket) - ADDR_SIZE;
}

//  public interface implementation
const sc_list_i List = {
    .new = list_new,
    .dispose = list_dispose,
    .capacity = list_capacity,
    .size = list_size,
    .append = list_append,
    .get = list_get_at,
    .remove = list_remove_at,
    .set = list_set_at,
    .insert = list_insert_at,
    .prepend = list_prepend,
    .clear = list_clear,
};
