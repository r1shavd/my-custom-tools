# Compiler and Flags
CC       := gcc
CFLAGS   := -std=c99 -D_GNU_SOURCE -Wall -Wextra -O3 -pthread

# Directories
SRC_DIR  := src
OBJ_DIR  := bin
TARGET   := bin/portscanner 

# Files
SRCS     := $(wildcard $(SRC_DIR)/*.c)
OBJS     := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Default Rule
all: $(TARGET)

# Link the Target
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Compile Object Files Directly into Bin Directory
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create Bin Directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean Build Files (Cleans objects without deleting the source)
clean:
	rm -f $(OBJS) $(TARGET)

# Declare Phony Targets
.PHONY: all clean
