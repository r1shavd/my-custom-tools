# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -Isrc -Ibuild/helpers
LIBS = -lcrypto

# Target Executable
TARGET = build/encrypter

# Source Directories
SRCDIR = src
BUILDDIR = build
MAIN_OBJ_DIR = $(BUILDDIR)/main
HELPERS_OBJ_DIR = $(BUILDDIR)/helpers

# Source and Object Files
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c, $(MAIN_OBJ_DIR)/%.o, $(SRCS))

# Default Rule
all: $(TARGET)

# Linking the Target (LIBS added at the end)
$(TARGET): $(OBJS) | $(BUILDDIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

# Compiling Source Files into Object Files
$(MAIN_OBJ_DIR)/%.o: $(SRCDIR)/%.c | $(MAIN_OBJ_DIR) $(HELPERS_OBJ_DIR)
	@# Move colors.h to the helpers folder if needed during the build
	@if [ ! -f $(HELPERS_OBJ_DIR)/colors.h ] && [ -f $(SRCDIR)/colors.h ]; then \
		cp $(SRCDIR)/colors.h $(HELPERS_OBJ_DIR)/; \
	fi
	$(CC) $(CFLAGS) -c $< -o $@

# Create directories if they do not exist
$(BUILDDIR) $(MAIN_OBJ_DIR) $(HELPERS_OBJ_DIR):
	mkdir -p $@

# Clean up build artifacts
clean:
	rm -rf $(BUILDDIR)

.PHONY: all clean
