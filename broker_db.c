/*
 * Broker database implementation through bash commands.
 */
#define _POSIX_C_SOURCE 2 // popen()

#include <stdio.h>
#include <stdlib.h>

#include "broker_db.h"
#include "system.h"

#define UNUSED(x) (void)(x)

#define DB_DIR "broker_data"

int db_init() {
    char *db_dir = DB_DIR;
    char command[1024];

    // Make directories
    snprintf(command, sizeof(command), "mkdir -p %1$s %1$s/topics %1$s/clients", db_dir);
    printf("db_init:\n\t+ %s\n", command);
    system(command);

    // Make file for ids
    snprintf(command, sizeof(command), "[ -s %1$s/last_id ] || echo 1 > %1$s/last_id", db_dir);
    printf("\t+ %s\n", command);
    return bash_exec(command);
}

long db_next_id() {
    char *db_dir = DB_DIR;
    char command[1024];

    // Make directories
    // sed -i "s/$(cat last_id)/$(($(cat last_id)+1))/" last_id
    snprintf(command, sizeof(command), "sed -i \"s/$(cat %1$s/last_id)/$(($(cat %1$s/last_id)+1))/\" %1$s/last_id && cat %1$s/last_id", db_dir);
    printf("db_next_id:\n\t+ %s\n", command);
    FILE* out = popen(command, "r");

    long new_id = -1;
    fscanf(out, "%ld", &new_id);
    pclose(out);

    return new_id;
}

int db_subscribe(long id, char *topic) {
    char *db_dir = DB_DIR;
    char command[1024];

    char topic_dir[100];
    snprintf(topic_dir, sizeof(topic_dir), "%s/topics/%s", db_dir, topic);

    char topic_file[100];
    snprintf(topic_file, sizeof(topic_file), "%s/_subscribers", topic_dir);

    char client_file[100];
    snprintf(client_file, sizeof(client_file), "%s/clients/%ld.subs", db_dir, id);

    // Create directories
    snprintf(command, sizeof(command), "mkdir -p %s", topic_dir);
    printf("db_subscribe:\n\t+ %s\n", command);
    int r = bash_exec(command);
    if (r < 0) {
        return r;
    }

    // Update subscribes
    snprintf(command, sizeof(command), "grep -q '^%1$ld' %2$s || echo '%1$ld' >> %2$s", id, topic_file);
    printf("\t+ %s\n", command);
    r = bash_exec(command);
    if (r < 0) {
        return r;
    }

    // Update subscribes
    snprintf(command, sizeof(command), "grep -q '^%1$s' %2$s || echo '%1$s' >> %2$s", topic, client_file);
    printf("\t+ %s\n", command);
    r = bash_exec(command);
    if (r < 0) {
        return r;
    }

    return 0;
}

int db_register_exit(long global_id, long exit_mtype) {
    char *db_dir = DB_DIR;
    char command[1024];

    // Create file with exit pid
    snprintf(command, sizeof(command), "echo '%ld' > %s/clients/%ld.exit", exit_mtype, db_dir, global_id);
    printf("\t+ %s\n", command);
    return bash_exec(command);
}

long db_get_exit(long global_id) {
    char *db_dir = DB_DIR;
    char command[1024];

    snprintf(command, sizeof(command), "cat %s/clients/%ld.exit", db_dir, global_id);

    long exit_mtype = -1;

    FILE* out = popen(command, "r");
    fscanf(out, "%ld", &exit_mtype);
    pclose(out);
    return exit_mtype;
}

int db_unsubscribe(long id, char *topic) {
    //    snprintf(command, sizeof(command), "grep -q '^%1$d' %3$s && sed -i 's/^%1$d.*/%1$d/' %3$s || echo '%1$d' >> %3$s", id, topic, topic_file);
    UNUSED(id);
    UNUSED(topic);
    return 0;
}

int db_get_subscriptors(char *topic, long **id_list) {
    char *db_dir = DB_DIR;
    char command[1024];
    char topic_file[100];
    snprintf(topic_file, sizeof(topic_file), "%s/topics/%s/_subscribers", db_dir, topic);
    snprintf(command, sizeof(command), "cat %s", topic_file);

    *id_list = (long*)malloc(sizeof(long) * 100);
    for (int i = 0; i < 100; i++) (*id_list)[i] = 0;
    FILE* out = popen(command, "r");
    int r;
    int i = 0;
    do {
        r = fscanf(out, "%ld\n", &(*id_list)[i]);
        if (r != EOF) i++;
    } while(r != EOF);
    pclose(out);
    return (i-1);
}

int db_store_message(char *topic, char *message, long publisher) {
    UNUSED(topic);
    UNUSED(message);
    UNUSED(publisher);
    return 0;
}

int db_close() {
    return 0;
}
