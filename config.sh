# Configuration file for build.sh
# Defines build variables sourced by the main build script

# Build system version
BUILD_SYSTEM_VERSION="1.1.0"

CC=gcc
STD=c2x

# Memory checking configuration (declared early for conditional compilation)
# VALGRIND_ENABLED: Set to true if valgrind is installed on the system
VALGRIND_ENABLED=true
# VALGRIND_OPTS: Default valgrind options
VALGRIND_OPTS="--leak-check=full --show-leak-kinds=all --track-origins=yes --verbose"

# ASAN_ENABLED: Set to true to compile with AddressSanitizer
ASAN_ENABLED=false
# ASAN_OPTIONS: Runtime options for AddressSanitizer
ASAN_OPTIONS="detect_leaks=1:detect_stack_use_after_return=1:detect_invalid_pointer_pairs=1"

# Base compiler flags
BASE_CFLAGS="-Wall -Wextra -g -fPIC -std=$STD -I./include"

# Add ASAN flags if enabled
if [ "$ASAN_ENABLED" = true ]; then
    BASE_CFLAGS="$BASE_CFLAGS -fsanitize=address -fsanitize=undefined"
fi

CFLAGS="$BASE_CFLAGS"
TST_CFLAGS="$CFLAGS -DTSTDBG -I/usr/include/sigmatest"
LDFLAGS="-shared"
TST_LDFLAGS="-lstest -L/usr/lib"

SRC_DIR=src
BUILD_DIR=build
BIN_DIR=bin
LIB_DIR="$BIN_DIR/lib"
TEST_DIR=test
TST_BUILD_DIR="$BUILD_DIR/test"

# Bundle definitions: space-separated list of source names (without .c)
COLLECTION_SOURCES="array_base collections list parray farray slotarray"

# Build target definitions: associative array mapping targets to commands
# See BUILDING.md for option details
declare -A BUILD_TARGETS=(
    ["all"]="build_lib"
    ["compile"]="compile_only"
    ["clean"]="clean"
    ["clean_all"]="clean_all"
    ["install"]="build_lib && install_lib"
    ["test"]="run_all_tests"
    ["root"]="show_project_info"
)

# Special test configurations: associative array mapping test names to linking strategies
# Requires SigmaTest for test sets
# See BUILDING.md for option details
declare -A TEST_CONFIGS=(
    ["test_setname"]="standard"
    ["setname"]="standard"
)

# Special test flags: space-separated list of compiler flags (without -D prefix)
# Flags will be prefixed with -D automatically
# See BUILDING.md for option details
declare -A TEST_COMPILE_FLAGS=(
    [setname]="TEST_BOOTSTRAP TEST_ISOLATION"
)
