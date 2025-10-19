# Makefile for mdu program
# Compiler and flags
CC = gcc
CFLAGS = -g -std=gnu11 -Werror -Wall -Wextra -Wpedantic -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition
LDFLAGS = -pthread

# Target executable
TARGET = mdu

# Source files
SOURCES = mdu.c dirsize.c work_queue.c
OBJECTS = $(SOURCES:.c=.o)
HEADERS = dirsize.h work_queue.h

# Default target
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

# Compile source files to object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)

# Phony targets
.PHONY: all clean
