#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "lb.h"
#include "msgqueue.h"

// Local broker deamon
int main(void) {
    printf("Starting broker deamon\n");

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
    int pid = fork();
    if (pid < 0) {
        perror("lb_daemon: fork");
    } else if (pid == 0) {
        execl("lb_sender", "lb_sender");
        perror("lb_daemon: execv:");
        _exit(-1);
    }

    for (int i = 0; i < 2; i++) {
        wait(NULL);
    }

    // Cleanup
    printf("Shutting down daemon\n");
    msgq_destroy(sendq);
    msgq_destroy(recvq);
    return 0;
}
