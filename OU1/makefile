# Compiler and compiler flags
CC = gcc
CFLAGS = -g -std=gnu11 -Werror -Wall -Wextra -Wpedantic -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition

# Flags for valgrind (windows/linux) and leaks (macOS)
VALGRINDFLAGS = valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose
LEAKSFLAGS = leaks --atExit --

# Input file, change at your discretion
FILENAME = in_file

# Object files to build
OBJS = mexec.o parse.o

mexec: $(OBJS)
	$(CC) $(CFLAGS) -o mexec $(OBJS)

mexec.o: mexec.c parse.h
	$(CC) $(CFLAGS) -c mexec.c

parse.o: parse.c parse.h
	$(CC) $(CFLAGS) -c parse.c

runwithoutfile: mexec
	./mexec

runwithfile: mexec
	./mexec < $(FILENAME)

runvalgrind: mexec
	$(VALGRINDFLAGS) ./mexec

runleaks: mexec
	$(LEAKSFLAGS) ./mexec

clean: 
	rm -f mexec *.o
