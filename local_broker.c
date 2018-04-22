#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>

#include "local_broker.h"
#include "msgqueue.h"

// Local broker deamon
int main(void) {
    printf("Starting broker deamon");

    // Create IPC queues for clients
    int sendq = msgq_create(LB_IPC_SEND_MQ);
    if (sendq < 0) {
        _exit(-1);
    }

    int recvq = msgq_create(LB_IPC_RECV_MQ);
    if (recvq < 0) {
        msgq_destroy(sendq);
        _exit(-1);
    }

    // Attempt to connect to the broker server

    // Launch process for send and receive to broker

    // Cleanup

    printf("Shutting down daemon");
    msgq_destroy(sendq);
    msgq_destroy(recvq);
    return 0;
}
