CC = gcc
CFLAGS = -g -std=gnu11 -Werror -Wall -Wextra -Wpedantic \
         -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition
LDFLAGS = -pthread

SRCS = mdu.c dirsize.c work_queue.c
OBJS = mdu.o dirsize.o work_queue.o

# Default target
all: mdu

# Link object files to create the executable
mdu: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o mdu

# Compile source files to object files
mdu.o: mdu.c dirsize.h
	$(CC) $(CFLAGS) -c mdu.c

dirsize.o: dirsize.c dirsize.h work_queue.h
	$(CC) $(CFLAGS) -c dirsize.c

work_queue.o: work_queue.c work_queue.h
	$(CC) $(CFLAGS) -c work_queue.c

# Clean up build artifacts
clean:
	rm -f mdu $(OBJS)
