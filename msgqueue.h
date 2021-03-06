#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H

// System V IPC message queue wrapper
#define _GNU_SOURCE

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

// TODO: make this customizable
#define ENV_KEY_PATH "KEY_PATH"
#define DEFAULT_KEY_PATH "./msgqueue.h"
#define DEFAULT_PERM 0644

/*
 * Attempts to create an IPC message queue with id _id_.
 * If the queue exists, it fails and returns -1.
 */
int msgq_create(int id) {
    char* key_path = getenv(ENV_KEY_PATH);
    if (!key_path) key_path = DEFAULT_KEY_PATH;

    key_t key = ftok(key_path, id);
    if (key < 0) {
        perror("msgq_create: ftok");
        return key;
    }

    int msgid = msgget(key, IPC_CREAT | IPC_EXCL | DEFAULT_PERM);
    if (msgid < 0) {
        perror("msgq_create: msgget");
    }
    return msgid;
}

/*
 * Attempts to retrieve the IPC message queue with id _id_.
 * If it does not exists, an error is returned.
 */
int msgq_getmsg(int id) {
    char* key_path = getenv(ENV_KEY_PATH);
    if (!key_path) key_path = DEFAULT_KEY_PATH;

    key_t key = ftok(key_path, id);
    if (key < 0) {
        perror("msgq_getmsg: ftok");
        return key;
    }

    int msgid = msgget(key, DEFAULT_PERM);
    if (msgid < 0) {
        perror("msgq_getmsg: msgget");
    }
    return msgid;
}

/*
 * Attempts to send a message to the queue with id _id_.
 *   - msgsize is the full length of msgp, including the
 * mtype long.
 */
int msgq_send(int msgid, const void *msgp, size_t msgsz) {
    int r = msgsnd(msgid, msgp, msgsz - sizeof(long), 0);
    if (r < 0) {
        perror("msgq_send: msgsnd");
    }
    return r;
}

/*
 * Attempts to read a message from the queue with id _id_, using
 * the provided mtype. The message is stored in msgp.
 */
int msgq_recv(int msgid, void *msgp, size_t msgsz, long mtype) {
    int r = msgrcv(msgid, msgp, msgsz - sizeof(long), mtype, 0);
    if (r < 0) {
        perror("msgq_recv: msgrcv");
    }
    return r;
}

/*
 * Attempts to destroy an IPC message queue with id _id_.
 */
int msgq_destroy(int msgid) {
    int r = msgctl(msgid, IPC_RMID, 0);
    if (r < 0) {
        perror("msgq_destroy: msgctl");
    }
    return r;
}

#endif // MSG_QUEUE_H
