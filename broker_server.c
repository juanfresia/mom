#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200112L // setenv()

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "broker_db.h"
#include "lb.h"
#include "log.h"
#include "msgqueue.h"
#include "proc.h"
#include "server.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT "12345"

void handler(socket_t* s) {
    static int connection_id = 1;
    log_printf("A connection has arrived\n");

    // Set environment variable for socket FD
    char buffer[BUFFER_LEN];
    sprintf(buffer, "%d", socket_get_fd(s));
    setenv(ENV_SOCKET_FD, buffer, 1);

    // Set environment variable for entrance/exit ID (mtype)
    sprintf(buffer, "%d", connection_id);
    setenv(ENV_CONNECTION_ID, buffer, 1);
    connection_id++;

    // Spawn broker entrance and exit for new connection
    // TODO: cleanup
    int pid = fork();
    if (pid < 0) {
        log_perror("broker_server: entrance fork");
    } else if (pid == 0) {
        execl("broker_entrance", "broker_entrance", NULL);
        log_perror("broker_server: execv");
        return;
    }

    pid = fork();
    if (pid < 0) {
        log_perror("broker_server: exit fork");
    } else if (pid == 0) {
        execl("broker_exit", "broker_exit", NULL);
        log_perror("broker_server: execv");
        return;
    }
}

int main (void) {
    // Init db
    if (db_init() < 0) {
        log_perror("broker_server: db_init");
        _exit(-1);
    }

    // Init broker processor queues
    int inq = msgq_create(B_IPC_IN_MQ);
    if (inq < 0) {
        _exit(-1);
    }

    int outq = msgq_create(B_IPC_OUT_MQ);
    if (outq < 0) {
        msgq_destroy(inq);
        _exit(-1);
    }

    // Launch process for broker processor
    int b_proc_pid = fork();
    if (b_proc_pid < 0) {
        log_perror("b_daemon: processor fork");
    } else if (b_proc_pid == 0) {
        execl("broker_processor", "broker_processor", NULL);
        log_perror("b_daemon: execl");
        _exit(-1);
    }

    // Run server accepting connections
    run_server(SERVER_IP, SERVER_PORT, handler, SERVER_ITERATIVE);

    // Server gracefully exited
    // Gracefully kill children
    int wpid = graceful_quit_or_kill(b_proc_pid, NULL);
    if (wpid != b_proc_pid) {
        log_perror("b_daemon: broker processor quit");
    }

    // Free resources
    msgq_destroy(inq);
    msgq_destroy(outq);

    // Close db
    db_close();
    return 0;
}
