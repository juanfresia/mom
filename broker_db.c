/*
 * Broker database implementation through bash commands.
 */
#define _POSIX_C_SOURCE 2 // popen()

#include <stdio.h>
#include <stdlib.h>

#include "broker_db.h"
#include "log.h"

#define UNUSED(x) (void)(x)

#define DEFAULT_DB_DIR "broker_data"
#define ENV_DB_DIR "DB_DIR"

int db_init() {
    char* db_dir = getenv(ENV_DB_DIR);
    if (!db_dir) db_dir = DEFAULT_DB_DIR;

    char command[1024];

    // Make directories
    snprintf(command, sizeof(command), "mkdir -p %1$s %1$s/topics %1$s/clients", db_dir);
    log_printf("db_init:\n\t+ %s\n", command);
    system(command);

    // Make file for ids
    snprintf(command, sizeof(command), "[ -s %1$s/last_id ] || echo 1 > %1$s/last_id", db_dir);
    log_printf("\t+ %s\n", command);
    return system(command);
}

long db_next_id() {
    char* db_dir = getenv(ENV_DB_DIR);
    if (!db_dir) db_dir = DEFAULT_DB_DIR;

    char command[1024];

    // Make directories
    // sed -i "s/$(cat last_id)/$(($(cat last_id)+1))/" last_id
    snprintf(command, sizeof(command), "sed -i \"s/$(cat %1$s/last_id)/$(($(cat %1$s/last_id)+1))/\" %1$s/last_id && cat %1$s/last_id", db_dir);
    log_printf("db_next_id:\n\t+ %s\n", command);
    FILE* out = popen(command, "r");

    long new_id = -1;
    fscanf(out, "%ld", &new_id);
    pclose(out);

    return new_id;
}

int db_subscribe(long id, char *topic) {
    char* db_dir = getenv(ENV_DB_DIR);
    if (!db_dir) db_dir = DEFAULT_DB_DIR;
    char command[1024];

    char topic_dir[100];
    snprintf(topic_dir, sizeof(topic_dir), "%s/topics/%s", db_dir, topic);

    char topic_file[100];
    snprintf(topic_file, sizeof(topic_file), "%s/_subscribers", topic_dir);

    char client_file[100];
    snprintf(client_file, sizeof(client_file), "%s/clients/%ld.subs", db_dir, id);

    // Create directories
    snprintf(command, sizeof(command), "mkdir -p %s", topic_dir);
    log_printf("db_subscribe:\n\t+ %s\n", command);
    int r = system(command);
    if (r < 0) {
        return r;
    }

    // Update subscribes
    snprintf(command, sizeof(command), "grep -q '^%1$ld$' %2$s || echo '%1$ld' >> %2$s", id, topic_file);
    log_printf("\t+ %s\n", command);
    r = system(command);
    if (r < 0) {
        return r;
    }

    // Update subscribes
    snprintf(command, sizeof(command), "grep -q '^%1$s$' %2$s || echo '%1$s' >> %2$s", topic, client_file);
    log_printf("\t+ %s\n", command);
    r = system(command);
    if (r < 0) {
        return r;
    }

    return 0;
}

int db_register_exit(long global_id, long exit_mtype) {
    char* db_dir = getenv(ENV_DB_DIR);
    if (!db_dir) db_dir = DEFAULT_DB_DIR;
    char command[1024];

    // Create file with exit pid
    snprintf(command, sizeof(command), "echo '%ld' > %s/clients/%ld.exit", exit_mtype, db_dir, global_id);
    log_printf("\t+ %s\n", command);
    return system(command);
}

long db_get_exit(long global_id) {
    char* db_dir = getenv(ENV_DB_DIR);
    if (!db_dir) db_dir = DEFAULT_DB_DIR;
    char command[1024];

    snprintf(command, sizeof(command), "cat %s/clients/%ld.exit", db_dir, global_id);

    long exit_mtype = -1;

    log_printf("\t+ %s\n", command);
    FILE* out = popen(command, "r");
    fscanf(out, "%ld", &exit_mtype);
    pclose(out);
    return exit_mtype;
}

int db_get_subscriptors(char *topic, long **id_list) {
    char* db_dir = getenv(ENV_DB_DIR);
    if (!db_dir) db_dir = DEFAULT_DB_DIR;
    char command[1024];
    char topic_file[100];
    snprintf(topic_file, sizeof(topic_file), "%s/topics/%s/_subscribers", db_dir, topic);
    snprintf(command, sizeof(command), "cat %s", topic_file);

    long* tmp_list = (long*)malloc(sizeof(long) * 100);
    for (int i = 0; i < 100; i++) tmp_list[i] = 0;

    log_printf("\t+ %s\n", command);
    FILE* out = popen(command, "r");
    int r;
    int i = 0;
    do {
        r = fscanf(out, "%ld\n", &(tmp_list[i]));
        if (r != EOF) i++;
    } while(r != EOF);
    pclose(out);

    *id_list = tmp_list;
    return i;
}

int db_get_subscriptions(long id, char ***topic_list) {
    char* db_dir = getenv(ENV_DB_DIR);
    if (!db_dir) db_dir = DEFAULT_DB_DIR;
    char command[1024];
    char client_file[100];
    snprintf(client_file, sizeof(client_file), "%s/clients/%ld.subs", db_dir, id);
    snprintf(command, sizeof(command), "cat %s", client_file);

    char** tmp_list = (char**)malloc(sizeof(char*) * 100);
    for (int i = 0; i < 100; i++) tmp_list[i] = malloc(sizeof(char) * 100);

    log_printf("\t+ %s\n", command);
    FILE* out = popen(command, "r");
    int r;
    int i = 0;
    do {
        r = fscanf(out, "%s\n", tmp_list[i]);
        if (r != EOF) i++;
    } while(r != EOF);
    pclose(out);

    *topic_list = tmp_list;
    return i;
}

int db_store_message(char *topic, char *message, long publisher) {
    UNUSED(topic);
    UNUSED(message);
    UNUSED(publisher);
    return 0;
}

void db_free_topic_list(char** topic_list) {
    for (int i = 0; i < 100; i++) free(topic_list[i]);
    free(topic_list);
}

int db_unsubscribe(long id, char *topic) {
    char* db_dir = getenv(ENV_DB_DIR);
    if (!db_dir) db_dir = DEFAULT_DB_DIR;
    char command[1024];

    char topic_file[100];
    snprintf(topic_file, sizeof(topic_file), "%s/topics/%s/_subscribers", db_dir, topic);

    char client_file[100];
    snprintf(client_file, sizeof(client_file), "%s/clients/%ld.subs", db_dir, id);

    // Delete client from topic
    // Note: using \# at the start of the sed to change delimiter
    // usually it would be sed -i '/pattern/d' file
    // but the delimiter (in this case /) can be change to whatever character
    // we want by using a backslash and that character at the begining.
    // This results in sed -i '\#patter#d' file, or in this case
    // sed -i "\ pattern d"
    snprintf(command, sizeof(command), "sed -i '\\ ^%1$ld$ d' %2$s", id, topic_file);
    log_printf("\t+ %s\n", command);
    int r = system(command);
    if (r < 0) {
        return r;
    }

    // Delete topic from client
    snprintf(command, sizeof(command), "sed -i '\\ ^%1$s$ d' %2$s", topic, client_file);
    log_printf("\t+ %s\n", command);
    r = system(command);
    if (r < 0) {
        return r;
    }

    return 0;
}

int db_unregister(long id) {
    char* db_dir = getenv(ENV_DB_DIR);
    if (!db_dir) db_dir = DEFAULT_DB_DIR;
    char command[1024];
    char client_prefix[100];

    char **topic_list;
    int count = db_get_subscriptions(id, &topic_list);
    for (int i = 0; i < count; i++) {
        db_unsubscribe(id, topic_list[i]);
    }
    db_free_topic_list(topic_list);

    snprintf(client_prefix, sizeof(client_prefix), "%s/clients/%ld", db_dir, id);
    snprintf(command, sizeof(command), "rm %1$s.subs %1$s.exit", client_prefix);
    log_printf("\t+ %s\n", command);
    int r = system(command);
    if (r < 0) {
        return r;
    }
    return 0;
}


int db_close() {
    return 0;
}
