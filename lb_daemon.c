#define _GNU_SOURCE

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "lb.h"
#include "lb_db.h"
#include "log.h"
#include "msgqueue.h"
#include "socket.h"

#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT "12345"
#define ENV_BROKER_IP "BROKER_IP"
#define ENV_BROKER_PORT "BROKER_PORT"

// Signal handler
#define UNUSED(x) (void)(x)
static int quit = 0;
void graceful_quit(int sig) {
    UNUSED(sig);
	quit = 1;
}

int sendq = -1;
int recvq = -1;
int pubq = -1;


void init_queues() {
    // Create IPC queues for clients
    sendq = msgq_create(LB_IPC_SEND_MQ);
    if (sendq < 0) {
        _exit(-1);
    }

    recvq = msgq_create(LB_IPC_RECV_MQ);
    if (recvq < 0) {
        msgq_destroy(sendq);
        _exit(-1);
    }

    // IPC queue for received messages
    pubq = msgq_create(LB_IPC_PUB_MQ);
    if (pubq < 0) {
        msgq_destroy(sendq);
        msgq_destroy(recvq);
        _exit(-1);
    }
}

void clean_queues() {
    if (sendq > 0) msgq_destroy(sendq);
    if (recvq > 0) msgq_destroy(recvq);
    if (pubq > 0) msgq_destroy(pubq);
}

// Local broker deamon
int main(void) {
    log_printf("Starting broker deamon\n");

    // Setting signal handler for graceful_quit
    struct sigaction sa = {0};
    sa.sa_handler = graceful_quit;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    // Init db and IPC queues
    init_directories();
    init_queues();

    // Retrieve broker ip and port form environment
    char* ip = getenv(ENV_BROKER_IP);
    if (!ip) ip = DEFAULT_IP;

    char* port = getenv(ENV_BROKER_PORT);
    if (!port) port = DEFAULT_PORT;

    // Attempt to connect to the broker server
    log_printf("Connecting to broker at %s:%s\n", ip, port);
    socket_t* s = socket_create(SOCK_ACTIVE);
    if (!s) {
        log_perror("lb_daemon: socket_create");
        clean_queues();
        _exit(-1);
    }

    if (socket_connect(s, ip, port) < 0) {
        log_perror("lb_daemon: socket_connect");
        socket_destroy(s);
        clean_queues();
        _exit(-1);
    }

    // Set environment variable for socket FD
    char buffer[BUFFER_LEN];
    sprintf(buffer, "%d", socket_get_fd(s));
    setenv(ENV_SOCKET_FD, buffer, 0);

    // Launch process for sending and receiving to broker
    int pid = fork();
    if (pid < 0) {
        log_perror("lb_daemon: sender fork");
        socket_destroy(s);
        clean_queues();
        _exit(-1);
    } else if (pid == 0) {
        execl("lb_sender", "lb_sender", NULL);
        log_perror("lb_daemon: execv");
        _exit(-1);
    }

    pid = fork();
    if (pid < 0) {
        log_perror("lb_daemon: receiver fork");
        socket_destroy(s);
        clean_queues();
        _exit(-1);
    } else if (pid == 0) {
        execl("lb_receiver", "lb_receiver", NULL);
        log_perror("lb_daemon: execv");
        _exit(-1);
    }

    // Only quit if SIGTERM is received
    // SIGTERM will be propagated to children processes
    while(!quit) {
        pause();
    }

    // Gracefully quit (wait for children to finish)
    for (int i = 0; i < 2; i++) {
        wait(NULL);
    }

    // Cleanup
    log_printf("Shutting down daemon\n");
    socket_destroy(s);
    clean_queues();
    return 0;
}
