# Compiler and flags
CC = gcc
CFLAGS = -Wall -g -fPIC -Iinclude  # Add -fPIC for shared library
LDFLAGS = -shared  # For linking .so
TST_CFLAGS = $(CFLAGS)
TST_LDFLAGS = -lsigtest -L/usr/lib

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin
LIB_DIR = $(BIN_DIR)/lib
TEST_DIR = test
TST_BUILD_DIR = $(BUILD_DIR)/test

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))
TST_SRCS = $(wildcard $(TEST_DIR)/*.c)
TST_OBJS = $(patsubst $(TEST_DIR)/%.c, $(TST_BUILD_DIR)/%.o, $(TST_SRCS))

# Targets
LIB_TARGET = $(LIB_DIR)/libsigcore.so
TST_TARGET = $(TST_BUILD_DIR)/run_tests

# Install paths (Ubuntu standard)
INSTALL_LIB_DIR = /usr/lib
INSTALL_INCLUDE_DIR = /usr/include

# Default target
all: $(LIB_TARGET)

# Build the shared library
$(LIB_TARGET): $(OBJS)
	@mkdir -p $(LIB_DIR)
	$(CC) $(OBJS) -o $(LIB_TARGET) $(LDFLAGS)

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile test source files
$(TST_BUILD_DIR)/%.o: $(TEST_DIR)/%.c
	@mkdir -p $(TST_BUILD_DIR)
	$(CC) $(TST_CFLAGS) -c $< -o $@

# Link all-tests executable
$(TST_TARGET): $(TST_OBJS) $(LIB_TARGET)
	@mkdir -p $(TST_BUILD_DIR)
	$(CC) $(TST_OBJS) -o $(TST_TARGET) $(TST_LDFLAGS) -lsigcore -L$(LIB_DIR)

# Single test target (e.g., test_list)
$(TST_BUILD_DIR)/test_%: $(TST_BUILD_DIR)/test_%.o $(LIB_TARGET)
	@mkdir -p $(TST_BUILD_DIR)
	$(CC) $< -o $@ $(TST_LDFLAGS) -lsigcore -L$(LIB_DIR)

# Install the library and header
install: $(LIB_TARGET)
	sudo cp $(LIB_TARGET) $(INSTALL_LIB_DIR)/
	sudo cp $(INCLUDE_DIR)/sigcore.h $(INSTALL_INCLUDE_DIR)/
	sudo ldconfig  # Update linker cache

# Run all tests
test: $(TST_TARGET)
	@$(TST_TARGET)

# Run single test (e.g., make test_list)
test_%: $(TST_BUILD_DIR)/test_%
	@$<

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: all clean install test test_%
