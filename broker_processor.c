#define _GNU_SOURCE

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "broker_db.h"
#include "lb.h"
#include "msgqueue.h"

#define UNUSED(x) (void)(x)

static int quit = 0;

void graceful_quit(int sig) {
    UNUSED(sig);
	quit = 1;
}

struct msg_t handle_message(struct msg_t req) {
    long *id_list;
    if (req.type == MSG_SUBSCRIBE) {
        printf("I received a subscribe\n");
        if (db_subscribe(req.global_id, req.topic) < 0) {
            req.type = MSG_ACK_ERROR;
            return req;
        }

        int count = db_get_subscriptors(req.topic, &id_list);
        printf("Subscribers list:");
        while (count > 0) {
            printf(" %ld", id_list[count]);
            count--;
        }
        printf("\n");
    } else if (req.type == MSG_REGISTER) {
        req.global_id = db_next_id();
        if (req.global_id <= 0) {
            req.global_id = 0;
            req.type = MSG_ACK_ERROR;
            return req;
        } else {
            req.type = MSG_NEW_ID;
            return req;
        }
    }
    req.type = MSG_ACK_OK;

    return req;
}

int main(void) {
    printf("Broker processor started!!\n");

    // Setting signal handler for graceful_quit
    struct sigaction sa = {0};
    sa.sa_handler = graceful_quit;
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);

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

    // Main work loop
    struct msg_t req;
    struct msg_t resp;
    while(!quit) {
        int r = msgq_recv(inq, &req, sizeof(struct msg_t), 0);
        if (r < 0) {
            perror("broker_processor: msgq_recv");
            continue;
        }
        resp = handle_message(req);
        r = msgq_send(outq, &resp, sizeof(struct msg_t));
        if (r < 0) {
            perror("broker_processor: msgq_send");
            continue;
        }
    }

    printf("Broker processor quits\n");
    return 0;
}
