// src/slot_array.c
/*
 * A sparse array is generally a data structure that stores non-zero elements
 * using a map where keys are indices and values are the non-zero elements. This
 * is a slot array that behaves like a sparse array, but it does not rely on a
 * mapping to store the non-zero elements.
 *
 * The slot array is a dynamic array of addr pointers, resized as needed. As items
 * are removed, the index is set to a `0` address (uintptr_t) to indicate that
 * it is empty. The primary difference between the slot array and the list is that
 * a list will compact when items are removed, while the slot array will not. The
 * slot array is designed to be the precursor to a sparse array.
 */

#include "sigcore.h"
#include "collections.h"
#include <stdlib.h>
#include <stdio.h>

/* Creates a new slot array with initial capacity, allocating space for capacity + 1 slots */
static slot_array slArr_new(int capacity)
{
   slot_array self = Mem.alloc(sizeof(struct slot_array_s));

   if (self)
   {
      self->bucket = Mem.alloc((capacity + 1) * ADDR_SIZE);

      if (!self->bucket)
      {
         Mem.free(self);
         return NULL;
      }
      self->end = (addr)self->bucket + capacity * ADDR_SIZE;
      self->count = 0;

      Collections.clear(self->bucket, self->end + ADDR_SIZE);
   }

   return self;
}
/* Returns the total number of slots available in the slot array */
static int slArr_getCapacity(slot_array self)
{
   return Collections.count(self->bucket, self->end);
}
/* Returns the count of items in the slot array */
static int slArr_getCount(slot_array self)
{
   return self->count;
}
/* Clear the slot array */
static void slArr_clear(slot_array self)
{
   if (self)
   {
      Collections.clear(self->bucket, self->end + ADDR_SIZE);
      self->count = 0;
   }
}
/* Add an item to the slot array */
static void slArr_addItem(slot_array self, object item)
{
   if (!self || !item)
      return;
   int capacity = slArr_getCapacity(self);
   int next = Collections.nextEmpty(self->bucket, self->end);

   if (next == -1)
   {
      capacity = capacity ? capacity * 2 : 4;
      addr *new_bucket = Mem.alloc((capacity + 1) * ADDR_SIZE);
      if (!new_bucket)
         return;

      Collections.copyTo(self->bucket, new_bucket, self->end);
      Collections.clear(new_bucket + slArr_getCapacity(self), (addr)new_bucket + (capacity + 1) * ADDR_SIZE);

      Mem.free(self->bucket);
      self->bucket = new_bucket;
      self->end = (addr)self->bucket + capacity * ADDR_SIZE;
      next = capacity / 2; // Use first new slot
   }

   self->bucket[next] = (addr)item;
   self->count++;
}
/* Try to get item at index */
static int slArr_tryGetItemAt(slot_array self, int index, object *item)
{
   if (!self || !item || index < 0 || index >= slArr_getCapacity(self))
   {
      *item = NULL;
      return 0;
   }
   *item = self->bucket[index] ? (object)self->bucket[index] : NULL;
   return *item != NULL;
}
/* Remove item from slot array */
static void slArr_removeItem(slot_array self, object item)
{
   if (!self || !item)
      return;

   int index = Collections.indexOf(self->bucket, self->end, item);
   if (index != -1)
   {
      self->bucket[index] = (addr)ADDR_EMPTY;
      self->count--;
   }
}
/* Frees the slot array's bucket and the structure itself */
static void slArr_free(slot_array self)
{
   if (self)
   {
      Mem.free(self->bucket);
      Mem.free(self);
   }
}

const ISlotArray SlotArray = {
    .new = slArr_new,
    .free = slArr_free,
    .add = slArr_addItem,
    .tryGetAt = slArr_tryGetItemAt,
    .remove = slArr_removeItem,
    .count = slArr_getCount,
    .capacity = slArr_getCapacity,
    .clear = slArr_clear,
    //.copyTo = NULL,     // haven't used yet ...
};
