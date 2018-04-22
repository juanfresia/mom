#define _POSIX_C_SOURCE 200112L
#define _XOPEN_SOURCE 500

#include "socket.h"
#include "server.h"
#include <unistd.h>
#include <errno.h>
#include <signal.h> // Sets signal handler for graceful quit
#include <sys/types.h>

#include <stdio.h>

static int quit = 0;

void graceful_quit(int sig) {
	quit = 1;
}

void iterative_handler(socket_t* s, handler_t callback) {
	callback(s);
	socket_destroy(s);
}

// Needs original socket in order to free it, otherwise the fork has to be done
// inside run_server.
void concurrent_handler(socket_t* s, handler_t callback, socket_t* orig) {
	int pid = fork();

	if (pid < 0) {
		printf("Fork error while handling connection!\n");
	} else if (pid == 0) {
		printf("I'm process %d handling a new request\n", getpid());
		callback(s);
		socket_destroy(s);
		socket_destroy(orig);
		printf("Process %d finish the request\n", getpid());
		_exit(0);
	}
	socket_destroy(s);
}

void run_server(char* ip, char* port, handler_t callback, server_type type) {
	sigset(SIGTERM, graceful_quit);
	sigset(SIGINT, graceful_quit);

	printf("Creating socket\n");
	socket_t* s = socket_create(SOCK_PASSIVE);
	if (!s) {
		printf("Error creating socket\n");
		return;
	}

	printf("Binding socket\n");
	int err = socket_bind(s, ip, port);
	if (err < 0) {
		perror("Binding failed");
		socket_destroy(s);
		return;
	}

	printf("Setting socket as passive\n");
	err = socket_listen(s, 0);
	if (err < 0) {
		printf("Listen failed: %d\n", err);
		socket_destroy(s);
		return;
	}

	while (errno != EINTR && !quit) {
		printf("Listening on socket %s:%s\n", ip, port);
		socket_t* s2 = socket_accept(s);
		if (!s2) {
			printf("Accept failed\n");
		} else {
			printf("Accept successful!\n");
			if (type == SERVER_CONCURRENT) {
				concurrent_handler(s2, callback, s);
			} else {
				iterative_handler(s2, callback);
			}
		}
	}
	printf("Exiting...\n");
	socket_destroy(s);
	return;
}
