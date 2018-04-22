#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>

#include "lb.h"
#include "msgqueue.h"

void handle_message(struct api_msg_t msg) {
    printf("Received a message of type [%s] and payload: %s\n", MSG_TYPE_TO_STRING(msg.type), msg.payload);
}

// Local broker deamon
int main(void) {
    printf("Starting local broker sender\n");

    // Get queue of messages to process and send to the broker
    int msgid = msgq_getmsg(LB_IPC_SEND_MQ);
    if (msgid < 0) {
        _exit(-1);
    }

    struct api_msg_t msg = {0};
    int r;
    for (int i = 0; i < 10; i++) {
        r = msgq_recv(msgid, &msg, sizeof(struct api_msg_t), 0);
        if (r < 0) {
            perror("lb_sender: msgq_recv");
            continue;
        }
        handle_message(msg);
    }

    // Cleanup
    printf("Shutting down broker sender\n");
    return 0;
}
