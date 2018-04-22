#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>

#include "lb.h"
#include "msgqueue.h"

// Local broker deamon
int main(void) {
    printf("Starting local broker receiver\n");

    // Get queue of messages to send to clients
    int msgid = msgq_getmsg(LB_IPC_RECV_MQ);
    if (msgid < 0) {
        _exit(-1);
    }

    // Cleanup
    printf("Shutting down broker receiver\n");
    return 0;
}
