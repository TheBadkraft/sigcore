.SILENT:

# =====================================================================
# SigmaCore v1.0.0 – Pure C2x – src/ + include/sigcore/
# =====================================================================
CC          = gcc
STD         = c2x
CFLAGS      = -Wall -Wextra -g -fPIC -std=$(STD) -I./include
TST_CFLAGS  = $(CFLAGS) -DTSTDBG -I/usr/include/sigmatest
LDFLAGS     = -shared
WRAP_LDFLAGS = -Wl,--wrap=malloc -Wl,--wrap=free -Wl,--wrap=calloc -Wl,--wrap=realloc
TST_LDFLAGS = -lstest -L/usr/lib $(WRAP_LDFLAGS)

SRC_DIR       = src
BUILD_DIR     = build
BIN_DIR       = bin
LIB_DIR       = $(BIN_DIR)/lib
TEST_DIR      = test
TST_BUILD_DIR = $(BUILD_DIR)/test

# ---------------------------------------------------------------------
# Sources & Objects
# ---------------------------------------------------------------------
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SOURCES))

TEST_SOURCES = $(filter-out $(TEST_DIR)/run_mem_functions.c, $(wildcard $(TEST_DIR)/*.c))
MEM_FUNCTIONS_SOURCE = $(TEST_DIR)/run_mem_functions.c
TEST_OBJECTS = $(patsubst $(TEST_DIR)/%.c, $(TST_BUILD_DIR)/%.o, $(TEST_SOURCES))
MEM_FUNCTIONS_OBJECT = $(TST_BUILD_DIR)/run_mem_functions.o

LIB_TARGET = $(LIB_DIR)/libsigcore.so

# =====================================================================
# Primary targets
# =====================================================================
all: $(LIB_TARGET)

$(LIB_TARGET): $(OBJECTS)
	mkdir -p $(LIB_DIR)
	if [ -n "$(OBJECTS)" ]; then \
	    $(CC) $(OBJECTS) -o $@ $(LDFLAGS); \
	else \
	    echo "Warning: No source files – creating empty shared library"; \
	    touch $@; \
	fi

# ---------------------------------------------------------------------
# Compile library sources (src/*.c → build/*.o)
# ---------------------------------------------------------------------
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# ---------------------------------------------------------------------
# Compile test sources
# ---------------------------------------------------------------------
$(TST_BUILD_DIR)/%.o: $(TEST_DIR)/%.c
	mkdir -p $(TST_BUILD_DIR)
	$(CC) $(TST_CFLAGS) -c $< -o $@

# ---------------------------------------------------------------------
# Individual test executables
# ---------------------------------------------------------------------
$(TST_BUILD_DIR)/test_%: $(TST_BUILD_DIR)/test_%.o $(OBJECTS)
	mkdir -p $(TST_BUILD_DIR)
	$(CC) $< $(OBJECTS) -o $@ $(TST_LDFLAGS)

# Run a single test
test_%: $(TST_BUILD_DIR)/test_%
	echo "=== Running $* ==="
	$<

# Run all tests
test: $(patsubst $(TEST_DIR)/test_%.c, test_%, $(TEST_SOURCES)) test_stack

# ---------------------------------------------------------------------
# Stack prototype test executable
# ---------------------------------------------------------------------
$(TST_BUILD_DIR)/test_stack: $(TST_BUILD_DIR)/prototyping/test_stack.o $(TST_BUILD_DIR)/prototyping/stack.o $(OBJECTS)
	mkdir -p $(TST_BUILD_DIR)
	$(CC) $(TST_BUILD_DIR)/prototyping/test_stack.o $(TST_BUILD_DIR)/prototyping/stack.o $(OBJECTS) -o $@ $(TST_LDFLAGS)

# Run stack prototype test
test_stack: $(TST_BUILD_DIR)/test_stack
	echo "=== Running stack prototype ==="
	$<

# ---------------------------------------------------------------------
# Prototyping directory compilation
# ---------------------------------------------------------------------
$(TST_BUILD_DIR)/prototyping/%.o: test/prototyping/%.c
	mkdir -p $(TST_BUILD_DIR)/prototyping
	$(CC) $(TST_CFLAGS) -c $< -o $@

# ---------------------------------------------------------------------
# Memory functions test executable
# ---------------------------------------------------------------------
$(TST_BUILD_DIR)/run_mem_functions: $(MEM_FUNCTIONS_OBJECT) $(OBJECTS)
	mkdir -p $(TST_BUILD_DIR)
	$(CC) $< $(OBJECTS) -o $@ $(TST_LDFLAGS)

# Run memory functions test
run_mem_functions: $(TST_BUILD_DIR)/run_mem_functions
	echo "=== Running memory functions test ==="
	$<

# =============================================================================
# Clean
# =============================================================================
clean:
	rm -f $(BUILD_DIR)/*.o $(TST_BUILD_DIR)/*.o $(TST_BUILD_DIR)/prototyping/*.o $(TST_BUILD_DIR)/test_*

clean_all: clean
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# =============================================================================
# Install
# =============================================================================
install: $(LIB_TARGET)
	sudo install -m 644 $(LIB_TARGET) /usr/lib/
	sudo mkdir -p /usr/include/sigcore
	sudo install -m 644 include/sigcore/*.h /usr/include/sigcore/
	sudo ldconfig

# =============================================================================
# Phony
# =============================================================================
.PHONY: all clean clean_all install test run_mem_functions