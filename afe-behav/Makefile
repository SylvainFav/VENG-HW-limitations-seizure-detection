
# Use GCC compiler
CC := gcc

# -g for debugging ; -Wall for all warnings
CFLAGS = -std=c99 -Wall -O3 -Ofast -g

SRCS := $(wildcard src/*.c) main.c

HEADERS := $(wildcard include/*.h)

BUILD_DIR := build

OBJS := $(SRCS:src/%.c=$(BUILD_DIR)/%.o)

TARGET := $(BUILD_DIR)/mainBehav

# The final target binary depends on object files
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -lm

# Rule for compiling .c to .o
$(BUILD_DIR)/%.o: src/%.c $(HEADERS)
	@mkdir -p $(BUILD_DIR)  # Create build directory if it doesn't exist
	$(CC) $(CFLAGS) -c $< -o $@

# Rule for running the program
run: $(TARGET)
	./$(TARGET)

# Rule for cleaning build files
clean:
	rm -rf $(BUILD_DIR)


	