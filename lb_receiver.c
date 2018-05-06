#define _GNU_SOURCE

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lb_db.h"
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
    log_printf("Listening for broker messages\n");

    int r = SOCK_RECV(s, struct msg_t, *msg);
    if (r < 0) {
        log_perror("get_message: sock_recv");
        _exit(-1);
    }

    log_printf("Received a message\n");
    print_msg(*msg);

    // If it is a response from a register, use the local_id
    // to set the mapping.
    if (msg->type == MSG_NEW_ID) {
        db_set_local_id(msg->local_id, msg->global_id);
    }
    msg->mtype = db_get_local_id(msg->global_id);

    return 0;
}

// Local broker deamon
int main(void) {
    log_printf("Starting local broker receiver\n");

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

    // Get queue for publish
    int pubid = msgq_getmsg(LB_IPC_PUB_MQ);
    if (pubid < 0) {
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
        log_printf("Sendin received message to queue\n");
        print_msg(msg);
        // Publishes are sent to another queue
        if (msg.type == MSG_PUBLISH) {
            r = msgq_send(pubid, &msg, sizeof(struct msg_t));
        } else {
            r = msgq_send(msgid, &msg, sizeof(struct msg_t));
        }
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
