#define _GNU_SOURCE

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "broker_db.h"
#include "lb.h"
#include "msgqueue.h"

#define UNUSED(x) (void)(x)

static int broker_id;
static int quit = 0;

void graceful_quit(int sig) {
    UNUSED(sig);
	quit = 1;
}

/*
 * Handles a request message req and returns a response.
 * As it may need to send multiple messages (e.g. in a publish)
 * the second parameter is the output queue.
 */
struct msg_t handle_message(struct msg_t req, int outq) {
    long *id_list;

    if (req.type == MSG_SUBSCRIBE) {
        log_printf("I received a subscribe\n");
        if (db_subscribe(req.global_id, req.topic) < 0) {
            req.type = MSG_ACK_ERROR;
            return req;
        }
    } else if (req.type == MSG_REGISTER) {
        log_printf("I received a register\n");
        req.global_id = db_next_id();
        if (req.global_id <= 0) {
            req.global_id = 0;
            req.type = MSG_ACK_ERROR;
            return req;
        }
        if (db_register_exit(req.global_id, req.mtype) < 0) {
            log_perror("registering exit");
        }
        req.type = MSG_NEW_ID;
        return req;
    } else if (req.type == MSG_PUBLISH) {
        log_printf("I received a publish\n");
        int count = db_get_subscriptors(req.topic, &id_list);
        log_printf("Publishing to %s\n", req.topic);

        struct msg_t new_msg = req;
        while (count > 0) {
            count--;
            new_msg.global_id = id_list[count];
            new_msg.mtype = db_get_exit(new_msg.global_id);
            if (new_msg.mtype <= 0) {
                log_printf("error finding exit for publish");
                continue;
            }

            if (new_msg.global_id == req.global_id && req.mtype != 1) {
                // Do not send back to publisher
                // if mtype == 1, then is from another broker
                // it could have the same id
                continue;
            }
            log_printf("\tnow sending to %ld\n", id_list[count]);
            print_msg(new_msg);
            int r = msgq_send(outq, &new_msg, sizeof(struct msg_t));
            if (r < 0) {
                log_perror("broker_processor: msgq_send");
                continue;
            }
        }
        log_printf("Finish sending\n");
        free(id_list);

        // If the publish came from a client and not a ring messages
        // TODO: define constant
        if (req.mtype != 1) {
            new_msg = req;
            new_msg.local_id = broker_id;
            new_msg.mtype = 1;
            log_printf("Sending publish to coordination queue\n");
            print_msg(new_msg);
            int r = msgq_send(outq, &new_msg, sizeof(struct msg_t));
            if (r < 0) {
                log_perror("broker_processor: msgq_send");
            }
        }
    } else if (req.type == MSG_UNSUBSCRIBE) {
        log_printf("I received an unsubscribe\n");
        if (db_unsubscribe(req.global_id, req.topic) < 0) {
            log_perror("unsubscribing");
        }
    } else if (req.type == MSG_UNREGISTER) {
        log_printf("I received an unregister\n");
        if (db_unregister(req.global_id) < 0) {
            log_perror("unregister");
        }
    }
    req.type = MSG_ACK_OK;
    return req;
}

int main(void) {
    log_printf("Broker processor started!!\n");

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

    // Retrieve broker id
    broker_id = 1;
    char* broker_id_str = getenv(ENV_BROKER_ID);
    if (!broker_id_str) {
        log_perror("BROKER_ID is not defined");
        broker_id = 0;
    } else {
        sscanf(broker_id_str, "%d", &broker_id);
    }

    // Main work loop
    struct msg_t req;
    struct msg_t resp;
    while(!quit) {
        log_printf("Waiting for messages to process\n");
        int r = msgq_recv(inq, &req, sizeof(struct msg_t), 0);
        if (r < 0) {
            log_perror("broker_processor: msgq_recv");
            continue;
        }
        log_printf("Got a new request\n");
        print_msg(req);

        resp = handle_message(req, outq);
        if (req.mtype == 1) continue;

        log_printf("Attempting to send response\n");
        print_msg(resp);
        r = msgq_send(outq, &resp, sizeof(struct msg_t));
        if (r < 0) {
            log_perror("broker_processor: msgq_send");
            continue;
        }
    }

    log_printf("Broker processor quits\n");
    return 0;
}
