#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lb.h"
#include "msgqueue.h"
#include "socket.h"

void handle_message(socket_t* s, struct api_msg_t msg) {
    printf("Received a message of type [%s] and payload: %s\n", MSG_TYPE_TO_STRING(msg.type), msg.payload);

    int r = SOCK_SEND(s, struct lb_msg_t, msg);
    if (r < 0) {
        perror("handle_message: sock_send");
    }
}

// Local broker deamon
int main(void) {
    printf("Starting local broker sender\n");

    // Get queue of messages to process and send to the broker
    int msgid = msgq_getmsg(LB_IPC_SEND_MQ);
    if (msgid < 0) {
        _exit(-1);
    }

    // Retrieve socket fd
    int sock_fd;
    sscanf(getenv("SOCKET_FD"), "%d", &sock_fd);
    socket_t* s = socket_create_from_fd(sock_fd, SOCK_ACTIVE);

    // Loop poping messages from local broker queue and sending to broker
    struct api_msg_t msg = {0};
    int r;
    for (int i = 0; i < 10; i++) {
        r = msgq_recv(msgid, &msg, sizeof(struct api_msg_t), 0);
        if (r < 0) {
            perror("lb_sender: msgq_recv");
            continue;
        }
        handle_message(s, msg);
    }

    // Cleanup
    socket_destroy(s);
    printf("Shutting down local broker sender\n");
    return 0;
}
