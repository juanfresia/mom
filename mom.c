#define _GNU_SOURCE
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

#define UNUSED(x) (void)(x)

/*
 * Initialize IPC message queues.
 * Returns MOM_ERROR if an error occurs, MOM_SUCCESS otherwise.
 * The msgids are stored in global variables msgid_snd and msgid_rcv.
 */
int init_msgq() {
    msgid_snd = msgq_getmsg(LB_IPC_SEND_MQ);
    if (msgid_snd < 0) {
        return MOM_ERROR;
    }

    msgid_rcv = msgq_getmsg(LB_IPC_RECV_MQ);
    if (msgid_rcv < 0) {
        return MOM_ERROR;
    }

    return MOM_SUCCESS;
}

// Implementation of MOM API

int register_client() {
    // Initialize IPC message queues
    if (init_msgq() < 0) {
        return MOM_ERROR;
    }

    // Send register message
    struct msg_t msg = {0};
    msg.mtype = getpid();
    msg.type = MSG_REGISTER;
    if (msgq_send(msgid_snd, &msg, sizeof(msg)) < 0) {
        return MOM_ERROR;
    }

    if (msgq_recv(msgid_rcv, &msg, sizeof(msg), msg.mtype) < 0) {
        return MOM_ERROR;
    }

    // Assert message type is a correct ACK
    if (msg.type != MSG_ACK_OK) {
        return MOM_ERROR;
    }

    // Return pid as id (TODO: change reading payload)
    return msg.mtype;
}

int subscribe(int id, char* topic) {
    // Send subscription message
    struct msg_t msg = {0};
    msg.mtype = id;
    msg.type = MSG_SUBSCRIBE;

    if (strlen(topic) >= MAX_TOPIC_LENGTH) {
        return MOM_ERROR;
    }
    strcpy(msg.topic, topic);

    if (msgq_send(msgid_snd, &msg, sizeof(msg)) < 0) {
        return MOM_ERROR;
    }
    // TODO: Expect suscription ack

    if (msgq_recv(msgid_rcv, &msg, sizeof(msg), msg.mtype) < 0) {
        return MOM_ERROR;
    }

    // Assert message type is a correct ACK
    if (msg.type != MSG_ACK_OK) {
        return MOM_ERROR;
    }

    return MOM_SUCCESS;
}

int unsubscribe(int id, char* topic) {
    UNUSED(id);
    UNUSED(topic);
    printf("to be implemented\n");
    return MOM_ERROR;
}

int publish(int id, char* topic, char* message) {
    // Send subscription message
    struct msg_t msg = {0};
    msg.mtype = id;
    msg.type = MSG_PUBLISH;

    if (strlen(topic) >= MAX_TOPIC_LENGTH) {
        return MOM_ERROR;
    }
    strcpy(msg.topic, topic);

    if (strlen(message) >= MAX_PAYLOAD) {
        return MOM_ERROR;
    }
    strcpy(msg.payload, message);

    if (msgq_send(msgid_snd, &msg, sizeof(msg)) < 0) {
        return MOM_ERROR;
    }
    // TODO: Expect publish ack
    if (msgq_recv(msgid_rcv, &msg, sizeof(msg), msg.mtype) < 0) {
        return MOM_ERROR;
    }

    // Assert message type is a correct ACK
    if (msg.type != MSG_ACK_OK) {
        return MOM_ERROR;
    }

    return MOM_SUCCESS;
}

int retrieve(int id, char* topic, char** msg_store) {
    UNUSED(id);
    UNUSED(topic);
    UNUSED(msg_store);
    printf("to be implemented\n");
    return MOM_ERROR;
}

int unregister(int id) {
    UNUSED(id);
    printf("to be implemented\n");
    return MOM_ERROR;
}
