#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include "lb.h"
#include "mom.h"
#include "msgqueue.h"

int send_msg(struct api_msg_t msg) {
    int msgid = msgq_getmsg(LB_IPC_SEND_MQ);
    if (msgid < 0) {
        perror("send_msg: unable to send message, queue may be invalid");
        return MOM_ERROR;
    }
    if (msgq_send(msgid, &msg, sizeof(msg)) < 0) {
        return MOM_ERROR;
    }
    return MOM_SUCCESS;
}

int register_client() {
    printf("to be implemented\n");
    return MOM_ERROR;
}

int subscribe(int id, char* topic) {
    struct api_msg_t msg = {0};
    msg.mtype = getpid();
    msg.type = MSG_SUBSCRIBE;

    return send_msg(msg);
}

int unsubscribe(int id, char* topic) {
    printf("to be implemented\n");
    return MOM_ERROR;
}

int publish(int id, char* topic, char* message) {
    printf("to be implemented\n");
    return MOM_ERROR;
}

int retrieve(int id, char* topic, char** msg_store) {
    printf("to be implemented\n");
    return MOM_ERROR;
}

int unregister(int id) {
    printf("to be implemented\n");
    return MOM_ERROR;
}
