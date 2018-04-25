#define _GNU_SOURCE

#include <signal.h>
#include <stdio.h>
#include <sys/types.h>

#include "lb.h"
#include "msgqueue.h"
#include "proc.h"
#include "server.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT "12345"

void handler(socket_t* s) {
    printf("A connection has arrived\n");

    // Get IPC queues
    int inq = msgq_getmsg(B_IPC_IN_MQ);
    if (inq < 0) {
        _exit(-1);
    }
    int outq = msgq_getmsg(B_IPC_OUT_MQ);
    if (outq < 0) {
        msgq_destroy(inq);
        _exit(-1);
    }

    struct lb_msg_t msg = {0};
    for (int i = 0; i < 10; i++) {
        printf("Waiting for requests\n");
        int r = SOCK_RECV(s, struct lb_msg_t, msg);
        if (r < 0) {
            perror("server_handler: sock_recv");
        }

        printf("I received a request [%lu, %s]\n", msg.global_id, MSG_TYPE_TO_STRING(msg.type));
        r = msgq_send(inq, &msg, sizeof(struct api_msg_t));
        if (r < 0) {
            perror("broker_processor: msgq_send");
            continue;
        }

        printf("Waiting for response processing\n");
        r = msgq_recv(outq, &msg, sizeof(struct api_msg_t), msg.global_id);
        if (r < 0) {
            perror("broker_processor: msgq_recv");
            continue;
        }

        printf("I received a response [%lu, %s]\n", msg.global_id, MSG_TYPE_TO_STRING(msg.type));
        r = SOCK_SEND(s, struct lb_msg_t, msg);
        if (r < 0) {
            perror("server_handler: sock_send");
        }
    }
}

int main (int argc, char* argv[]) {
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
        perror("b_daemon: processor fork");
    } else if (b_proc_pid == 0) {
        execl("broker_processor", "broker_processor", NULL);
        perror("b_daemon: execl");
        _exit(-1);
    }

    // Run server accepting connections
    run_server(SERVER_IP, SERVER_PORT, handler, SERVER_CONCURRENT);

    // Server gracefully exited
    // Gracefully kill children
    int wpid = graceful_quit_or_kill(b_proc_pid, NULL);
    if (wpid != b_proc_pid) {
        perror("b_daemon: broker processor quit");
    }

    // Free resources
    msgq_destroy(inq);
    msgq_destroy(outq);
    return 0;
}
