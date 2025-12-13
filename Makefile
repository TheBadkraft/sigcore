.SILENT:

# =====================================================================
# SigmaCore v1.0.0 – Pure C2x – src/ + include/sigcore/
# =====================================================================
CC          = gcc
STD         = c2x
CFLAGS      = -Wall -Wextra -g -fPIC -std=$(STD) -Iinclude
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

TEST_SOURCES = $(wildcard $(TEST_DIR)/*.c)
TEST_OBJECTS = $(patsubst $(TEST_DIR)/%.c, $(TST_BUILD_DIR)/%.o, $(TEST_SOURCES))

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
test: $(patsubst $(TEST_DIR)/test_%.c, test_%, $(TEST_SOURCES))

# =============================================================================
# Clean
# =============================================================================
clean:
	rm -f $(BUILD_DIR)/*.o $(TST_BUILD_DIR)/*.o $(TST_BUILD_DIR)/test_*

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
.PHONY: all clean clean_all install test