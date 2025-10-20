CC = gcc
CFLAGS = -g -std=gnu11 -Wall -Wextra -Wpedantic -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition
LDFLAGS = -pthread
TARGET = mdu

SRC = mdu.c dirsize.c work_queue.c
OBJ = $(SRC:.c=.o)
DEPS = dirsize.h work_queue.h

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean