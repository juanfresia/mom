#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "lb.h"
#include "mom.h"
#include "msgqueue.h"

// Global variables for IPC message queues
int msgid_snd = -1;
int msgid_rcv = -1;
int msgid_pub = -1;

#define UNUSED(x) (void)(x)

/*
 * Initialize IPC message queues.
 * Returns MOM_ERROR if an error occurs, MOM_SUCCESS otherwise.
 * The msgids are stored in global variables msgid_snd and msgid_rcv.
 */
int init_msgq() {
    log_printf("Initializing msg_snd\n");
    msgid_snd = msgq_getmsg(LB_IPC_SEND_MQ);
    if (msgid_snd < 0) {
        return MOM_ERROR;
    }

    log_printf("Initializing msg_rcv\n");
    msgid_rcv = msgq_getmsg(LB_IPC_RECV_MQ);
    if (msgid_rcv < 0) {
        return MOM_ERROR;
    }

    log_printf("Initializing msg_pub\n");
    msgid_pub = msgq_getmsg(LB_IPC_PUB_MQ);
    if (msgid_pub < 0) {
        return MOM_ERROR;
    }

    return MOM_SUCCESS;
}

/*
 * Sends a message to the local broker, and expects a response.
 * The response overwrites in the sent message.
 * On error MOM_ERROR is returned, otherwise it returns MOM_SUCCESS
 */
int send_msg_expect_response(struct msg_t *msg) {
    log_printf("Sending message to local broker\n");
    print_msg(*msg);
    if (msgq_send(msgid_snd, msg, sizeof(*msg)) < 0) {
        return MOM_ERROR;
    }

    log_printf("Waiting for ack...\n");
    if (msgq_recv(msgid_rcv, msg, sizeof(*msg), msg->mtype) < 0) {
        return MOM_ERROR;
    }
    log_printf("Received ack from local broker\n");
    print_msg(*msg);
    return MOM_SUCCESS;
}

/*
 * Implementation of MOM API
 */


int register_client() {
    // Initialize IPC message queues
    if (init_msgq() < 0) {
        return MOM_ERROR;
    }

    // Send register message
    struct msg_t msg = {0};
    msg.mtype = getpid();
    msg.type = MSG_REGISTER;
    if (send_msg_expect_response(&msg) == MOM_ERROR) {
        return MOM_ERROR;
    }

    // Assert message type is a new ID
    if (msg.type != MSG_NEW_ID) {
        return MOM_ERROR;
    }

    // Return asigned local ID
    return msg.local_id;
}

int subscribe(int id, char* topic) {
    // Build subscription message
    struct msg_t msg = {0};
    msg.mtype = id;
    msg.type = MSG_SUBSCRIBE;

    if (strlen(topic) >= MAX_TOPIC_LENGTH) {
        log_printf("subscribe: topic too long");
        return MOM_ERROR;
    }
    strcpy(msg.topic, topic);

    // Sending message
    if (send_msg_expect_response(&msg) == MOM_ERROR) {
        return MOM_ERROR;
    }

    // Assert message type is a correct ACK
    if (msg.type != MSG_ACK_OK) {
        return MOM_ERROR;
    }

    return MOM_SUCCESS;
}

int unsubscribe(int id, char* topic) {
    // Send subscription message
    struct msg_t msg = {0};
    msg.mtype = id;
    msg.type = MSG_UNSUBSCRIBE;

    if (strlen(topic) >= MAX_TOPIC_LENGTH) {
        log_printf("unsubscribe: topic too long");
        return MOM_ERROR;
    }
    strcpy(msg.topic, topic);

    // Sending message
    if (send_msg_expect_response(&msg) == MOM_ERROR) {
        return MOM_ERROR;
    }

    // Assert message type is a correct ACK
    if (msg.type != MSG_ACK_OK) {
        return MOM_ERROR;
    }

    return MOM_SUCCESS;
}

int publish(int id, char* topic, char* message) {
    // Send subscription message
    struct msg_t msg = {0};
    msg.mtype = id;
    msg.type = MSG_PUBLISH;

    if (strlen(topic) >= MAX_TOPIC_LENGTH) {
        log_printf("publish: topic too long");
        return MOM_ERROR;
    }
    strcpy(msg.topic, topic);

    if (strlen(message) >= MAX_PAYLOAD) {
        log_printf("publish: payload too long");
        return MOM_ERROR;
    }
    strcpy(msg.payload, message);

    // Sending message
    if (send_msg_expect_response(&msg) == MOM_ERROR) {
        return MOM_ERROR;
    }

    // Assert message type is a correct ACK
    if (msg.type != MSG_ACK_OK) {
        return MOM_ERROR;
    }

    return MOM_SUCCESS;
}

int retrieve(int id, char** msg_store) {
    struct msg_t msg = {0};

    log_printf("Attemt to retreive a message\n");
    if (msgq_recv(msgid_pub, &msg, sizeof(msg), id) < 0) {
        return MOM_ERROR;
    }
    log_printf("Received a message\n");
    print_msg(msg);

    *msg_store = malloc(sizeof(char) * (strlen(msg.payload) + 1));
    strcpy(*msg_store, msg.payload);

    return MOM_SUCCESS;
}

int unregister(int id) {
    // Send subscription message
    struct msg_t msg = {0};
    msg.mtype = id;
    msg.type = MSG_UNREGISTER;

    log_printf("Sending unregister\n");
    print_msg(msg);
    if (msgq_send(msgid_snd, &msg, sizeof(msg)) < 0) {
        return MOM_ERROR;
    }

    log_printf("Waiting for unregister ack\n");
    if (msgq_recv(msgid_rcv, &msg, sizeof(msg), msg.mtype) < 0) {
        return MOM_ERROR;
    }
    log_printf("Received unregister ack\n");
    print_msg(msg);

    // Assert message type is a correct ACK
    if (msg.type != MSG_ACK_OK) {
        return MOM_ERROR;
    }
    return MOM_SUCCESS;
}
