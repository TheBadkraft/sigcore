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
 * File: scope.h
 * Description: Header file for SigmaCore scope transfer operations
 */
#pragma once

#include "sigcore/types.h"

/**
 * @brief Transfer ownership of an object from one scope to another.
 *
 * IMPORTANT RESTRICTIONS:
 * =======================================================================
 * 🚫 MEMORY SCOPE TRANSFERS ARE NOT SUPPORTED 🚫
 *
 * Objects allocated with Memory.alloc() CANNOT be transferred to other scopes.
 * This is due to incompatible allocation models:
 * - Memory: individual malloc/free with slotarray tracking
 * - Arena/Frame: bump allocation within pre-allocated pages
 *
 * Attempting to transfer Memory objects will result in undefined behavior
 * or memory corruption. Memory objects must be disposed in their original scope.
 * =======================================================================
 *
 * Supported transfers:
 * ✅ Arena ↔ Arena
 * ✅ Frame ↔ Arena
 * ✅ Frame ↔ Frame
 *
 * @param from Source scope (arena or frame)
 * @param to   Destination scope (arena or frame)
 * @param obj  Object to transfer
 * @return 0 on success, -1 on failure
 */
int scope_move_scopes(void *from, void *to, object obj);