#ifndef LOCAL_BROKER_H
#define LOCAL_BROKER_H

#define LB_IPC_SEND_MQ 0
#define LB_IPC_RECV_MQ 1

#define MAX_PAYLOAD 1024
#define MAX_TOPIC_LENGTH 100
#define MAX_MESSAGE_LENGTH MAX_PAYLOAD-MAX_TOPIC_LENGTH

#define BUFFER_LEN 100

#define ENV_SOCKET_FD "SOCKET_FD"

enum mom_msg_type {
    MSG_REGISTER = 0,
    MSG_SUBSCRIBE,
    MSG_UNSUBSCRIBE,
    MSG_PUBLISH,
    MSG_RETRIEVE,
    MSG_UNREGISTER,
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
    "ack_ok",
    "ack_error"
};

#define MSG_TYPE_TO_STRING(type) (msg_type_string[type])

// Message structure for communication with the broker
struct lb_msg_t {
    long global_id;
    enum mom_msg_type type;
    char payload[MAX_PAYLOAD];
};

// Message for communication
struct api_msg_t {
    long mtype;
    enum mom_msg_type type;
    char payload[MAX_PAYLOAD];
};

// Payload types, cast the payload to this types

// MSG_SUBSCRIBE, MSG_UNSUBSCRIBE, MSG_PUBLISH, MSG_RETRIEVE
struct pyld_subs_t {
    char topic[MAX_TOPIC_LENGTH];
    char message[MAX_MESSAGE_LENGTH];
};

// MSG_REGISTER
struct pyld_reg_t {
    long id;
    char padding[MAX_PAYLOAD - sizeof(long)];
};


#endif // LOCAL_BROKER_H
