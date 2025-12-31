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
 * File: page_tests.h
 * Description: Internal header for arena page testing backdoors
 */
#pragma once

#include "sigcore/slotarray.h"
#include "sigcore/types.h"

// Forward declarations for testing
typedef struct sc_page sc_page;

// Backdoor functions for testing arena pages
// These provide access to internal page state for validation

/**
 * @brief Create a new arena page with specified data capacity
 * @param data_size Size of the data area in bytes
 * @return Pointer to the created page, or NULL on failure
 */
sc_page *Page_create(usize data_size);

/**
 * @brief Destroy an arena page and free all tracked allocations
 * @param page The page to destroy
 */
void Page_destroy(sc_page *page);

/**
 * @brief Allocate memory from a page with optional zero initialization
 * @param page The page to allocate from
 * @param size Size of allocation in bytes
 * @param zero If true, zero-initialize the allocated memory
 * @return Pointer to allocated memory, or NULL if allocation fails
 */
object Page_alloc(sc_page *page, usize size, bool zero);

/**
 * @brief Get the current bump pointer position in the page
 * @param page The page to query
 * @return Current bump pointer
 */
void *Page_get_bump(sc_page *page);

/**
 * @brief Get the number of bytes used in the page
 * @param page The page to query
 * @return Bytes used
 */
usize Page_get_used(sc_page *page);

/**
 * @brief Get the total data capacity of the page
 * @param page The page to query
 * @return Data capacity in bytes
 */
usize Page_get_capacity(sc_page *page);

/**
 * @brief Get the slotarray tracking allocations in this page
 * @param page The page to query
 * @return The tracking slotarray
 */
slotarray Page_get_tracked_addrs(sc_page *page);

/**
 * @brief Check if a pointer is within this page's data area
 * @param page The page to check
 * @param ptr The pointer to verify
 * @return true if pointer is within page data area
 */
bool Page_contains(sc_page *page, object ptr);

/**
 * @brief Get the number of tracked allocations in the page
 * @param page The page to query
 * @return Number of tracked allocations
 */
usize Page_get_allocation_count(sc_page *page);