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

int get_message(socket_t *s, struct msg_t *msg) {
    printf("Attempt to get a message from broker\n");

    int r = SOCK_RECV(s, struct msg_t, *msg);
    if (r < 0) {
        perror("get_message: sock_recv");
        return -1;
    }

    printf("Received a message from broker of type [%s] and payload: %s\n", MSG_TYPE_TO_STRING(msg->type), msg->payload);
    return 0;
}

// Local broker deamon
int main(void) {
    printf("Starting local broker receiver\n");

    // Setting signal handler for graceful_quit
    struct sigaction sa = {0};
    sa.sa_handler = graceful_quit;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    // Get queue of messages to send to clients
    int msgid = msgq_getmsg(LB_IPC_RECV_MQ);
    if (msgid < 0) {
        _exit(-1);
    }

    // Retrieve socket fd
    int sock_fd;
    sscanf(getenv(ENV_SOCKET_FD), "%d", &sock_fd);
    socket_t* s = socket_create_from_fd(sock_fd, SOCK_ACTIVE);

    // Loop reading messages from broker and pushing into local broker queue
    struct msg_t msg = {0};
    int r;
    while(!quit) {
        if (get_message(s, &msg) < 0) {
            _exit(-1);
        }
        msg.mtype = msg.global_id;
        r = msgq_send(msgid, &msg, sizeof(struct msg_t));
        if (r < 0) {
            perror("lb_receiver: msgq_send");
            _exit(-1);
        }
    }

    // Cleanup
    printf("Shutting down local broker receiver\n");
    socket_destroy(s);
    return 0;
}
