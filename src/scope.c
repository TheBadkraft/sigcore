/*
 * SigmaCore
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
 * File: scope.c
 * Description: SigmaCore scope transfer implementation
 */
#include "sigcore/scope.h"
#include "internal/arena_internal.h"
#include "internal/memory_internal.h"
#include "sigcore/arena.h"
#include "sigcore/memory.h"
#include <string.h>

// Forward declarations for internal structs
struct sc_frame;

// Check if scope is Arena
static bool is_arena_scope(void *scope) {
   if (!scope)
      return false;
   const char *handle = (const char *)scope;
   return memcmp(handle, "ARN", 4) == 0;
}

// Check if scope is Frame
static bool is_frame_scope(void *scope) {
   if (!scope)
      return false;
   const char *handle = (const char *)scope;
   return memcmp(handle, "FRM", 4) == 0;
}

// Add object to scope tracking
static int scope_add_object(void *scope, object obj) {
   if (!scope || !obj)
      return ERR;

   if (is_arena_scope(scope)) {
      Arena.track((arena)scope, obj);
      return OK;
   } else if (is_frame_scope(scope)) {
      // Frames use arena tracking
      arena frame_arena = frame_get_arena((frame)scope);
      if (frame_arena) {
         Arena.track(frame_arena, obj);
         return OK;
      }
      return ERR;
   }

   return ERR;
}

// Remove object from scope tracking
static int scope_remove_object(void *scope, object obj) {
   if (!scope || !obj)
      return ERR;

   if (is_arena_scope(scope)) {
      if (Arena.is_tracking((arena)scope, obj)) {
         Arena.untrack((arena)scope, obj);
         return OK;
      }
      return ERR; // Not owned by this scope
   } else if (is_frame_scope(scope)) {
      // Frames use arena tracking
      arena frame_arena = frame_get_arena((frame)scope);
      if (frame_arena && Arena.is_tracking(frame_arena, obj)) {
         Arena.untrack(frame_arena, obj);
         return OK;
      }
      return ERR; // Not owned by this scope
   }

   return ERR;
}

// Transfer ownership between scopes
int scope_move_scopes(void *from, void *to, object obj) {
   if (!from || !to || !obj)
      return ERR;

   // Validate scope types (only arenas and frames supported)
   bool from_valid = is_arena_scope(from) || is_frame_scope(from);
   bool to_valid = is_arena_scope(to) || is_frame_scope(to);

   if (!from_valid || !to_valid) {
      return ERR;
   }

   // Remove from source scope
   if (scope_remove_object(from, obj) != OK) {
      return ERR; // Object not owned by source
   }

   // Add to destination scope
   if (scope_add_object(to, obj) != OK) {
      // Rollback - add back to source
      scope_add_object(from, obj);
      return ERR;
   }

   return OK;
}

// Import external data into a scope
object scope_import(void *scope, const void *data, usize size) {
   if (!scope || !data || size == 0)
      return NULL;

   if (is_arena_scope(scope)) {
      // Import into arena - allocate and copy
      object ptr = Arena.alloc((arena)scope, size, false);
      if (!ptr)
         return NULL;
      memcpy(ptr, data, size);
      return ptr;
   } else if (is_frame_scope(scope)) {
      // Import into frame's arena - allocate and copy
      arena frame_arena = frame_get_arena((frame)scope);
      if (!frame_arena)
         return NULL;
      object ptr = Arena.alloc(frame_arena, size, false);
      if (!ptr)
         return NULL;
      memcpy(ptr, data, size);
      return ptr;
   }

   // Unsupported scope type
   return NULL;
}