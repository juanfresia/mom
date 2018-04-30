#define _GNU_SOURCE

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lb.h"
#include "msgqueue.h"
#include "socket.h"

#define UNUSED(x) (void)(x)

static int quit = 0;

void graceful_quit(int sig) {
    UNUSED(sig);
	quit = 1;
}

int main(void) {
    printf("Broker exit started!!\n");

    // Setting signal handler for graceful_quit
    struct sigaction sa = {0};
    sa.sa_handler = graceful_quit;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    // Get IPC queue
    int outq = msgq_getmsg(B_IPC_OUT_MQ);
    if (outq < 0) {
        perror("broker_exit: msgq_getmsg");
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
        printf("Waiting for response processing\n");
        int r = msgq_recv(outq, &msg, sizeof(struct msg_t), connection_id);
        if (r < 0) {
            perror("broker_processor: msgq_recv");
            _exit(-1);
        }

        printf("I'm going to send a response [%lu, %s]\n", msg.global_id, MSG_TYPE_TO_STRING(msg.type));
        r = SOCK_SEND(s, struct msg_t, msg);
        if (r < 0) {
            perror("server_handler: sock_send");
            _exit(-1);
        }
    }

    printf("Broker exit finishes!!\n");
    return 0;
}
