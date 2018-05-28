CC= gcc
CFLAGS= -g --std=c11 -Wall -Wextra
NON-MAIN= mom.c server.c socket.c broker_db.c log.c lb_db.c
BINARIES= $(filter-out $(NON-MAIN), $(wildcard *.c))

all: clean $(BINARIES:.c=)

$(BINARIES:.c=): log.o
client: client.c mom.o socket.o
broker_server: broker_server.c socket.o server.o broker_db.o
broker_processor: broker_processor.c broker_db.o
broker_entrance: broker_entrance.c socket.o
broker_exit: broker_exit.c socket.o
broker_coord: broker_coord.c socket.o
lb_daemon: lb_daemon.c socket.o lb_db.o
lb_sender: lb_sender.c socket.o lb_db.o
lb_receiver: lb_receiver.c socket.o lb_db.o
	$(CC) $(CFLAGS) $^ -o $@

clean:
	ipcrm -a
	rm -rf *.o $(BINARIES:.c=) broker_data* lb_data*

.PHONY: clean
