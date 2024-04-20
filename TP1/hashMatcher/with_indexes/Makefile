CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99 -fopenmp -Wno-deprecated-declarations
LIBS = -lcrypto

SRC = hash_comparer_with_indexes.c

all: program

program: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o program $(LIBS)

clean:
	rm -f program
