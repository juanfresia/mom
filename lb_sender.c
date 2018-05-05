#define _GNU_SOURCE
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lb.h"
#include "lb_db.h"
#include "msgqueue.h"
#include "socket.h"

#define UNUSED(x) (void)(x)

static int quit = 0;

void graceful_quit(int sig) {
    UNUSED(sig);
	quit = 1;
}

void handle_message(socket_t* s, struct msg_t msg) {
    log_printf("Received a message from local queue\n");
    print_msg(msg);

    // msg.mtype is the connection ID except on register
    // save it so it is posible to know to whom return it
    // in case of a register
    msg.local_id = msg.mtype;

    if (msg.type != MSG_REGISTER) {
        msg.global_id = get_global_id(msg.mtype);
    }

    log_printf("Attempting to send to broker\n");
    print_msg(msg);
    int r = SOCK_SEND(s, struct msg_t, msg);
    if (r < 0) {
        log_perror("handle_message: sock_send");
        _exit(-1);
    }
}

// Local broker deamon
int main(void) {
    log_printf("Starting local broker sender\n");

    // Setting signal handler for graceful_quit
    struct sigaction sa = {0};
    sa.sa_handler = graceful_quit;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    // Get queue of messages to process and send to the broker
    int msgid = msgq_getmsg(LB_IPC_SEND_MQ);
    if (msgid < 0) {
        _exit(-1);
    }

    // Retrieve socket fd
    int sock_fd;
    sscanf(getenv(ENV_SOCKET_FD), "%d", &sock_fd);
    socket_t* s = socket_create_from_fd(sock_fd, SOCK_ACTIVE);

    // Loop poping messages from local broker queue and sending to broker
    struct msg_t msg = {0};
    int r;
    while(!quit) {
        log_printf("Waiting for messages in local queue\n");
        r = msgq_recv(msgid, &msg, sizeof(struct msg_t), 0);
        if (r < 0) {
            log_perror("lb_sender: msgq_recv");
            continue;
        }
        handle_message(s, msg);
    }

    // Cleanup
    socket_destroy(s);
    printf("Shutting down local broker sender\n");
    return 0;
}
