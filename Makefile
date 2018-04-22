CC= gcc
CFLAGS= -g --std=c11 -Wall -Wextra
NON-MAIN= mom.c
BINARIES= $(filter-out $(NON-MAIN), $(wildcard *.c))

all: clean $(BINARIES:.c=)

client: client.c mom.o
	$(CC) $(CFLAGS) $^ -o $@

clean:
	ipcrm -a
	rm -rf *.o $(BINARIES:.c=)

.PHONY: clean
