#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "lb.h"
#include "lb_db.h"
#include "msgqueue.h"
#include "socket.h"

// Local broker deamon
int main(void) {
    printf("Starting broker deamon\n");

    init_directories();
    set_local_id(10, 1003);
    set_local_id(11, 1241);

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
    socket_t* s = socket_create(SOCK_ACTIVE);
    if (!s) {
        perror("lb_daemon: socket_create");
        msgq_destroy(sendq);
        msgq_destroy(recvq);
        _exit(-1);
    }

    if (socket_connect(s, "127.0.0.1", "12345") < 0) {
        perror("lb_daemon: socket_connect");
        socket_destroy(s);
        msgq_destroy(sendq);
        msgq_destroy(recvq);
        _exit(-1);
    }

    // Set environment variable for socket FD
    char buffer[BUFFER_LEN];
    sprintf(buffer, "%d", socket_get_fd(s));
    setenv(ENV_SOCKET_FD, buffer, 0);

    // Launch process for sending and receiving to broker
    int pid = fork();
    if (pid < 0) {
        perror("lb_daemon: sender fork");
    } else if (pid == 0) {
        execl("lb_sender", "lb_sender", NULL);
        perror("lb_daemon: execv");
        _exit(-1);
    }

    pid = fork();
    if (pid < 0) {
        perror("lb_daemon: receiver fork");
    } else if (pid == 0) {
        execl("lb_receiver", "lb_receiver", NULL);
        perror("lb_daemon: execv");
        _exit(-1);
    }

    for (int i = 0; i < 2; i++) {
        wait(NULL);
    }

    // Cleanup
    printf("Shutting down daemon\n");
    socket_destroy(s);
    msgq_destroy(sendq);
    msgq_destroy(recvq);
    return 0;
}
