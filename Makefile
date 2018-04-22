CC= gcc
CFLAGS= -g --std=c11 -Wall -Wextra --pedantic

all: clean local_broker

local_broker: local_broker.o msgqueue.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o local_broker

.PHONY: clean
