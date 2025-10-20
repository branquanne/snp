CC = gcc
CFLAGS = -g -std=gnu11 -Werror -Wall -Wextra -Wpedantic -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition

# Flags for valgrind (windows/linux) and leaks (macOS)
VALGRINDFLAGS = valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose
LEAKSFLAGS = leaks --atExit --

OBJS = mmake.o parser.o

mmake: $(OBJS)
	$(CC) $(CFLAGS) -o mmake $(OBJS)

mmake.o: mmake.c parser.h
	$(CC) $(CFLAGS) -c mmake.c

parser.o: parser.c parser.h
	$(CC) $(CFLAGS) -c parser.c

runwithvalgrind: mmake
	$(VALGRINDFLAGS) ./mmake mexec

runwithleaks: mmake
	$(LEAKSFLAGS) ./mmake mexec

clean:
	rm -f mmake *.o