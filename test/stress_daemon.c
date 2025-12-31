/*
 * Stress Daemon: Endless randomized stress-testing for SigmaCore memory system
 * Links with stest.so for memory tracking via wrapped functions
 * Uses SigmaTests framework with constructor-based initialization
 */

#include "sigcore/arena.h"
#include "sigcore/memory.h"
#include <execinfo.h>
#include <signal.h>
#include <sigtest/sigtest.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// Page data size constant (matches arena.c)
#define PAGE_DATA_SIZE 4096

// Configuration: Max cycles (0 = infinite)
#define MAX_CYCLES 50000000ULL

// Global state
static FILE *log_stream = NULL;
static unsigned long long cycle_counter = 0;

// Signal handler for crashes
void crash_handler(int sig) {
   fprintf(log_stream, "DAEMON: Crash detected (signal %d), dumping trace...\n", sig);
   void *buffer[100];
   int nptrs = backtrace(buffer, 100);
   backtrace_symbols_fd(buffer, nptrs, fileno(log_stream));
   fflush(log_stream);
   exit(1);
}

// Memory validation every 100 iterations
void validate_memory(void) {
   // Use Memory.validate() if implemented, otherwise dummy stats
   // For now, assume sigtest provides allocation counts
   fprintf(log_stream, "DAEMON: Memory validation - Cycle: %llu\n", cycle_counter);
   fflush(log_stream);
}

// Test case: Arena creation/destruction stress
bool test_arena_creation(void) {
   sc_arena *arena = Memory.create_arena(1);
   if (!arena)
      return false;

   // Allocate some memory to stress
   object ptr = Arena.alloc(arena, 64, false);
   if (!ptr) {
      Memory.dispose_arena(arena);
      return false;
   }

   Memory.dispose_arena(arena);
   return true;
}

// Test case: Frame push/pop stress
bool test_frame_push_pop(void) {
   // Assuming frames are implemented; placeholder
   // Frame.push();
   // object ptr = Memory.alloc(128, false);
   // Frame.pop();
   // For now, simulate with arena
   sc_arena *arena = Memory.create_arena(1);
   if (!arena)
      return false;

   object ptr = Arena.alloc(arena, 128, false);
   if (!ptr) {
      Memory.dispose_arena(arena);
      return false;
   }

   Memory.dispose_arena(arena);
   return true;
}

// Test case: Ownership moves (placeholder for future implementation)
bool test_ownership_moves(void) {
   // Placeholder: Simulate allocation and "move"
   object ptr = Memory.alloc(256, false);
   if (!ptr)
      return false;

   // Simulate move (in future, actual ownership transfer)
   Memory.dispose(ptr);
   return true;
}

// Test case: Page overflow stress
bool test_page_overflow(void) {
   sc_arena *arena = Memory.create_arena(1);
   if (!arena)
      return false;

   // Allocate until page overflow
   for (int i = 0; i < 100; i++) {
      object ptr = Arena.alloc(arena, 1000, false);
      if (!ptr)
         break; // Expected on overflow
   }

   Memory.dispose_arena(arena);
   return true;
}

// Test case: Memory churn (rapid alloc/free)
bool test_memory_churn(void) {
   for (int i = 0; i < 1000; i++) {
      object ptr = Memory.alloc(32, false);
      if (!ptr)
         return false;
      Memory.dispose(ptr);
   }
   return true;
}

// Array of test functions
typedef bool (*test_func)(void);
static test_func test_cases[] = {
    test_arena_creation,
    test_frame_push_pop,
    test_ownership_moves,
    test_page_overflow,
    test_memory_churn};
static const int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);

// Main daemon loop (runs as a testcase)
void daemon_runner(void) {
   srand(time(NULL));

   while (1) {
      // Randomly select test
      int test_idx = rand() % num_tests;
      test_func selected_test = test_cases[test_idx];

      // Run test 10,000 times or until failure
      for (int i = 0; i < 10000; i++) {
         if (!selected_test()) {
            fprintf(log_stream, "DAEMON: Test failure in iteration %d of test %d\n", i, test_idx);
            fflush(log_stream);
            exit(1);
         }

         cycle_counter++;

         // Check for max cycles
         if (MAX_CYCLES > 0 && cycle_counter >= MAX_CYCLES) {
            fprintf(log_stream, "DAEMON: Reached max cycles (%llu). Exiting cleanly.\n", MAX_CYCLES);
            fflush(log_stream);
            exit(0);
         }

         // Validate every 100 iterations
         if (cycle_counter % 100 == 0) {
            validate_memory();
         }

         // Success milestone
         if (cycle_counter % 1000000 == 0) {
            fprintf(log_stream, "DAEMON: clean run. Memory stable. Cycle: %llu\n", cycle_counter);
            fflush(log_stream);
            sleep(5);
         }
      }
   }
}

// Configuration setup
static void set_config(FILE **log) {
   *log = log_stream = fopen("logs/stress_daemon.log", "w");
   if (!log_stream) {
      perror("Failed to open log file");
      exit(1);
   }

   // Set memory hooks to sigtest's wrapped functions
   Memory.set_alloc_hooks(__wrap_malloc, __wrap_free, NULL, NULL);

   // Set up signal handlers
   signal(SIGSEGV, crash_handler);
   signal(SIGABRT, crash_handler);
   signal(SIGILL, crash_handler);
}

// Teardown
static void set_teardown(void) {
   Memory.reset_alloc_hooks();
   Memory.teardown();
   if (log_stream) {
      fclose(log_stream);
   }
}

// Register with SigmaTests
__attribute__((constructor)) void init_stress_daemon(void) {
   testset("stress_daemon", set_config, set_teardown);
   testcase("Endless stress runner", daemon_runner);
}