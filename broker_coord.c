#define _GNU_SOURCE

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lb.h"
#include "log.h"
#include "msgqueue.h"
#include "socket.h"

#define ENV_BROKER_NEXT_IP "NEXT_IP"
#define ENV_BROKER_NEXT_PORT "NEXT_PORT"

#define ENV_BROKER_IP "BROKER_IP"
#define ENV_BROKER_PORT "BROKER_PEER_PORT"

#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT "12346"

#define UNUSED(x) (void)(x)

static int quit = 0;

void graceful_quit(int sig) {
    UNUSED(sig);
	quit = 1;
}

// Global IPC queues
static int inq = -1;
static int outq = -1;
static int coorq = -1;

// Broker ID
static int broker_id;
static int connection_id = 1;

void spawn_coord_entrance() {
    // Retrieve next broker ip and port form environment
    char* ip = getenv(ENV_BROKER_IP);
    if (!ip) ip = DEFAULT_IP;

    char* port = getenv(ENV_BROKER_PORT);
    if (!port) port = DEFAULT_PORT;

    log_printf("Broker %d: Creating socket for previous\n", broker_id);
	socket_t* s = socket_create(SOCK_PASSIVE);
	if (!s) {
		log_printf("Error creating socket\n");
		return;
	}

	log_printf("Broker %d: Binding socket\n", broker_id);
	int err = socket_bind(s, ip, port);
	if (err < 0) {
		log_perror("Binding failed");
		socket_destroy(s);
		return;
	}
	err = socket_listen(s, 0);
	if (err < 0) {
		log_printf("Listen failed: %d\n", err);
		socket_destroy(s);
		return;
	}

    // Attempt one accept
	log_printf("Listening on socket %s:%s\n", ip, port);
	socket_t* s2 = socket_accept(s);
	if (!s2) {
		log_printf("Accept failed\n");
        return;
	}
	log_printf("Accept successful!\n");

    socket_destroy(s);

    // Set environment variable for socket FD
    char buffer[BUFFER_LEN];
    sprintf(buffer, "%d", socket_get_fd(s2));
    setenv(ENV_SOCKET_FD, buffer, 1);

    // Set envoronment variables for entrance queue
    sprintf(buffer, "%d", B_IPC_COORD_MQ);
    setenv(ENV_QUEUE_ID, buffer, 1);

    // Spawn broker entrance and exit for new connection
    int pid = fork();
    if (pid < 0) {
        log_perror("broker_server: entrance fork");
    } else if (pid == 0) {
        execl("broker_entrance", "broker_entrance", NULL);
        log_perror("broker_server: execv");
        return;
    }
}

void spawn_coord_exit() {
    // Retrieve next broker ip and port form environment
    char* ip = getenv(ENV_BROKER_NEXT_IP);
    if (!ip) ip = DEFAULT_IP;

    char* port = getenv(ENV_BROKER_NEXT_PORT);
    if (!port) port = DEFAULT_PORT;

    // Attempt to connect to the broker server
    log_printf("Broker %d: Connecting to next broker at %s:%s\n", broker_id, ip, port);
    socket_t* s = socket_create(SOCK_ACTIVE);
    if (!s) {
        log_perror("broker_coord_exit: socket_create");
        _exit(-1);
    }

    if (socket_connect(s, ip, port) < 0) {
        log_perror("broker_coord_exit: socket_connect");
        socket_destroy(s);
        _exit(-1);
    }

    // Set environment variable for socket FD
    char buffer[BUFFER_LEN];
    sprintf(buffer, "%d", socket_get_fd(s));
    setenv(ENV_SOCKET_FD, buffer, 1);

    // Set envoronment variables for exit queue
    sprintf(buffer, "%d", B_IPC_OUT_MQ);
    setenv(ENV_QUEUE_ID, buffer, 1);

    // Spawn exit
    int pid = fork();
    if (pid < 0) {
        log_perror("broker_server: exit fork");
    } else if (pid == 0) {
        execl("broker_exit", "broker_exit", NULL);
        log_perror("broker_server: execv");
        return;
    }
}

void coordinate() {
    log_printf("Coordinator %d: entered working loop\n", broker_id);

    // Main work loop
    struct msg_t req;
    while(!quit) {
        log_printf("Waiting for messages to process\n");
        int r = msgq_recv(coorq, &req, sizeof(struct msg_t), 0);
        if (r < 0) {
            log_perror("broker_coordinator: msgq_recv");
            _exit(-1);
        }
        log_printf("Got a new request\n");
        print_msg(req);

        // If the message completed a cicle, remove it from the ring
        if (req.local_id == broker_id) {
            continue;
        } else if (req.type == MSG_PUBLISH) {
            r = msgq_send(inq, &req, sizeof(struct msg_t));
            if (r < 0) {
                log_perror("broker_coordinator: msgq_send replicating publish");
            }
        }

        r = msgq_send(outq, &req, sizeof(struct msg_t));
        if (r < 0) {
            log_perror("broker_coordinator: msgq_send");
            _exit(-1);
        }
    }
}

int main(void) {
    log_printf("Broker coordinator started!!\n");

    // Setting signal handler for graceful_quit
    struct sigaction sa = {0};
    sa.sa_handler = graceful_quit;
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);

    // Get IPC in queue to send publishes
    inq = msgq_getmsg(B_IPC_IN_MQ);
    if (inq < 0) {
        log_perror("broker_coordinator: msgq_getmsg");
        _exit(-1);
    }

    // Get IPC in queue to send coordinator mesages
    outq = msgq_getmsg(B_IPC_OUT_MQ);
    if (outq < 0) {
        log_perror("broker_coordinator: msgq_getmsg");
        _exit(-1);
    }

    // Get IPC in queue of control plane
    coorq = msgq_getmsg(B_IPC_COORD_MQ);
    if (coorq < 0) {
        log_perror("broker_coordinator: msgq_getmsg");
        _exit(-1);
    }

    // Retrieve broker id
    broker_id = 1;
    char* broker_id_str = getenv(ENV_BROKER_ID);
    if (!broker_id_str) {
        log_perror("BROKER_ID is not defined");
        broker_id = 0;
    } else {
        sscanf(broker_id_str, "%d", &broker_id);
    }

    char buffer[BUFFER_LEN];
    sprintf(buffer, "%d", connection_id);
    setenv(ENV_CONNECTION_ID, buffer, 1);

    // Spawn receiver and sender processes.
    if (broker_id == MASTER_ID) {
        spawn_coord_exit();
        spawn_coord_entrance();
    } else {
        spawn_coord_entrance();
        spawn_coord_exit();
    }

    coordinate();

    log_printf("Broker coordinator exiting\n");
    return 0;
}
