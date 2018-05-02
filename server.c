#define _POSIX_C_SOURCE 200112L
#define _XOPEN_SOURCE 500

#include "log.h"
#include "socket.h"
#include "server.h"
#include <unistd.h>
#include <errno.h>
#include <signal.h> // Sets signal handler for graceful quit
#include <sys/types.h>

#include <stdio.h>

static int quit = 0;

void graceful_quit(int sig __attribute__((unused))) {
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
		log_printf("Fork error while handling connection!\n");
	} else if (pid == 0) {
		log_printf("I'm process %d handling a new request\n", getpid());
		callback(s);
		socket_destroy(s);
		socket_destroy(orig);
		log_printf("Process %d finish the request\n", getpid());
		_exit(0);
	}
	socket_destroy(s);
}

void run_server(char* ip, char* port, handler_t callback, server_type type) {
    // Setting signal handler for graceful_quit
    struct sigaction sa = {0};
    sa.sa_handler = graceful_quit;
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);

	log_printf("Creating socket\n");
	socket_t* s = socket_create(SOCK_PASSIVE);
	if (!s) {
		log_printf("Error creating socket\n");
		return;
	}

	log_printf("Binding socket\n");
	int err = socket_bind(s, ip, port);
	if (err < 0) {
		log_perror("Binding failed");
		socket_destroy(s);
		return;
	}

	log_printf("Setting socket as passive\n");
	err = socket_listen(s, 0);
	if (err < 0) {
		log_printf("Listen failed: %d\n", err);
		socket_destroy(s);
		return;
	}

	while (errno != EINTR && !quit) {
		log_printf("Listening on socket %s:%s\n", ip, port);
		socket_t* s2 = socket_accept(s);
		if (!s2) {
			log_printf("Accept failed\n");
		} else {
			log_printf("Accept successful!\n");
			if (type == SERVER_CONCURRENT) {
				concurrent_handler(s2, callback, s);
			} else {
				iterative_handler(s2, callback);
			}
		}
	}
	log_printf("Exiting...\n");
	socket_destroy(s);
	return;
}
