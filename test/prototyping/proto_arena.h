/*
 * SigmaCore Prototype Arena Test
 * Custom memory allocation hooks for testing
 */

#ifndef PROTO_ARENA_H
#define PROTO_ARENA_H

#include <stddef.h>

// Fixed pointer values for testing
#define PROTO_PTR_1 ((void *)0x1000)
#define PROTO_PTR_2 ((void *)0x2000)
#define PROTO_PTR_3 ((void *)0x3000)
#define PROTO_PTR_4 ((void *)0x4000)

// Custom allocation functions that return fixed pointers
void *proto_malloc(size_t size);
void proto_free(void *ptr);
void *proto_calloc(size_t nmemb, size_t size);
void *proto_realloc(void *ptr, size_t size);

// Test setup function
void proto_setup_hooks(void);

// Test verification functions
int proto_verify_allocations(void);
void proto_reset_tracking(void);

#endif // PROTO_ARENA_H