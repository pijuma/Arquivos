# Compiler and flags
CC = gcc
CFLAGS = -std=c99 -Wall -Werror -g -Iinclude

# Directories
SRC_DIR = src
BIN_DIR = bin
INCLUDE_DIR = include

# Source and object files
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%.o, $(SRC_FILES))

# Output executable
OUTPUT = $(BIN_DIR)/programaTrab

# Default target
all: $(OUTPUT)

# Link object files to create executable
$(OUTPUT): $(OBJ_FILES)
	$(CC) $(CFLAGS) $^ -o $@

# Pattern rule for compiling source files to object files
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -rf $(BIN_DIR)

# Run the program
run: $(OUTPUT)
	./$(OUTPUT)

# Debug the program with Valgrind
debug: $(OUTPUT)
	valgrind --leak-check=yes --show-leak-kinds=all --log-file=valgrind.rpt ./$(OUTPUT)

# Zip the project files
zip:
	zip -r main.zip $(INCLUDE_DIR) $(SRC_DIR) Makefile
