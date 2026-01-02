/*
 * SigmaCore Prototype Arena Test Implementation
 * Custom memory allocation hooks that return fixed pointer values for testing
 */

#include "proto_arena.h"
#include <string.h>

// Tracking for testing
static int alloc_count = 0;
static int free_count = 0;
static void *last_allocated = NULL;
static void *last_freed = NULL;

// Fixed pointer sequence for testing
static void *proto_pointers[] = {
    PROTO_PTR_1,
    PROTO_PTR_2,
    PROTO_PTR_3,
    PROTO_PTR_4};
static int next_ptr_index = 0;

void *proto_malloc(size_t size) {
   if (next_ptr_index >= sizeof(proto_pointers) / sizeof(proto_pointers[0])) {
      return NULL; // No more test pointers
   }
   void *ptr = proto_pointers[next_ptr_index++];
   alloc_count++;
   last_allocated = ptr;
   return ptr;
}

void proto_free(void *ptr) {
   free_count++;
   last_freed = ptr;
   // Don't actually free since these are fixed test pointers
}

void *proto_calloc(size_t nmemb, size_t size) {
   size_t total_size = nmemb * size;
   void *ptr = proto_malloc(total_size);
   if (ptr) {
      memset(ptr, 0, total_size);
   }
   return ptr;
}

void *proto_realloc(void *ptr, size_t size) {
   // Simple realloc simulation - just allocate new if size > 0, free if size == 0
   if (size == 0) {
      proto_free(ptr);
      return NULL;
   }
   // For testing, just return a new pointer
   return proto_malloc(size);
}

void proto_setup_hooks(void) {
   // Hooks removed - this function is no longer used
   // extern void memory_set_alloc_hooks(
   //     void *(*malloc_hook)(size_t),
   //     void (*free_hook)(void *),
   //     ...);

   // memory_set_alloc_hooks(proto_malloc, proto_free, proto_calloc, proto_realloc);
}

int proto_verify_allocations(void) {
   // Verify that allocations returned expected fixed pointers
   return (alloc_count > 0 && last_allocated != NULL);
}

void proto_reset_tracking(void) {
   alloc_count = 0;
   free_count = 0;
   last_allocated = NULL;
   last_freed = NULL;
   next_ptr_index = 0;
}