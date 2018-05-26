#define _GNU_SOURCE

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lb.h"
#include "log.h"
#include "msgqueue.h"
#include "socket.h"

#define UNUSED(x) (void)(x)

static int quit = 0;

void graceful_quit(int sig) {
    UNUSED(sig);
	quit = 1;
}

int main(void) {
    log_printf("Broker entrance started!!\n");

    // Setting signal handler for graceful_quit
    struct sigaction sa = {0};
    sa.sa_handler = graceful_quit;
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);

    // Get IPC queue
    int queue_id;
    sscanf(getenv(ENV_QUEUE_ID), "%d", &queue_id);
    log_printf("Using queue_id %d\n", queue_id);
    int inq = msgq_getmsg(queue_id);
    if (inq < 0) {
        log_perror("broker_entrance: msgq_getmsg");
        _exit(-1);
    }

    // Retrieve socket fd
    int sock_fd;
    sscanf(getenv(ENV_SOCKET_FD), "%d", &sock_fd);
    socket_t* s = socket_create_from_fd(sock_fd, SOCK_ACTIVE);

    // Retrieve connection id
    int connection_id;
    sscanf(getenv(ENV_CONNECTION_ID), "%d", &connection_id);

    // Main loop
    struct msg_t msg = {0};
    while(!quit) {
        log_printf("Waiting for requests\n");
        int r = SOCK_RECV(s, struct msg_t, msg);
        if (r <= 0) {
            log_perror("server_handler: sock_recv");
            _exit(-1);
        }
        msg.mtype = connection_id;

        log_printf("Received a request\n");
        print_msg(msg);

        r = msgq_send(inq, &msg, sizeof(struct msg_t));
        if (r < 0) {
            log_perror("broker_processor: msgq_send");
            _exit(-1);
        }
    }

    log_printf("Broker entrance exiting\n");
    return 0;
}
