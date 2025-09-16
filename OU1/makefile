CC = gcc
CFLAGS = -g -std=gnu11 -Werror -Wall -Wextra -Wpedantic -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition -o
VALGRINDFLAGS = valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose
LEAKSFLAGS = leaks --atExit --

OBJS = mexec.c parse.c

mexec: $(OBJS)
	$(CC) $(CFLAGS) mexec $(OBJS)

runvalgrind: mexec
	$(VALGRINDFLAGS) ./mexec

runleaks: mexec
	$(LEAKSFLAGS) ./mexec

clean: 
	rm -f mexec *.o
