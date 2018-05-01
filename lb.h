#ifndef LOCAL_BROKER_H
#define LOCAL_BROKER_H

#include <stdio.h>
#include "log.h"

#define LB_IPC_SEND_MQ 0
#define LB_IPC_RECV_MQ 1

#define B_IPC_IN_MQ 2
#define B_IPC_OUT_MQ 3

#define MAX_PAYLOAD 1024
#define MAX_TOPIC_LENGTH 100

#define BUFFER_LEN 100

#define ENV_SOCKET_FD "SOCKET_FD"
#define ENV_CONNECTION_ID "CONNECTION_ID"

enum mom_msg_type {
    MSG_REGISTER = 0,
    MSG_SUBSCRIBE,
    MSG_UNSUBSCRIBE,
    MSG_PUBLISH,
    MSG_RETRIEVE,
    MSG_UNREGISTER,
    MSG_NEW_ID,
    MSG_ACK_OK,
    MSG_ACK_ERROR,
    MSG_ENUM_LENGTH
};
const char* msg_type_string[MSG_ENUM_LENGTH] = {
    "register",
    "subscribe",
    "unsubcribe",
    "publish",
    "read",
    "unregister",
    "new_id",
    "ack_ok",
    "ack_error"
};

#define MSG_TYPE_TO_STRING(type) (msg_type_string[type])

// Message structure for communication with the broker
struct msg_t {
    long mtype;
    long global_id;
    long local_id;
    enum mom_msg_type type;
    char topic[MAX_TOPIC_LENGTH];
    char payload[MAX_PAYLOAD];
};

void print_msg(struct msg_t msg) {
    log_printf("(mtype: %ld, lid: %ld, gid: %ld, type: %s, topic: %s, payload: %s)",
               msg.mtype, msg.local_id, msg.global_id, MSG_TYPE_TO_STRING(msg.type),
               msg.topic, msg.payload);
}

#endif // LOCAL_BROKER_H
