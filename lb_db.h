/*
 * Local broker database.
 * It manages local clients and maps its global_id to its local_id.
 * There are _N_ basic functions:
 *  - set_local_id(int local_id, long global_id)
 *  - get_local_id(long global_id)
 *  - get_global_id(int local_id)
 */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L // setenv()
#endif

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "log.h"

char* db_dir = "lb_data";

/*
 * Initialize local directories where to store the id mappings.
 * If the environment variable LB_DATABASE_DIR is set, it is used,
 * otherwise the value is defaulted to LB_DATABASE_DIR_DEFAULT.
 */
int init_directories() {
    char command[1024];
    snprintf(command, sizeof(command), "mkdir -p %s %s/global %s/local", db_dir, db_dir, db_dir);
    log_printf("+ %s\n", command);

    int r = system(command);
    if (r < 0) {
        log_perror("lb_db: init_directories");
        return r;
    }
    return 0;
}

int set_local_id(long local_id, long global_id) {
    char command[1024];
    snprintf(command, sizeof(command), "echo '%lu' > %s/local/%lu", local_id, db_dir, global_id);
    log_printf("+ %s\n", command);

    int r = system(command);
    if (r < 0) {
        log_perror("lb_db: set_local_id");
        return r;
    }

    snprintf(command, sizeof(command), "echo '%lu' > %s/global/%lu", global_id, db_dir, local_id);
    log_printf("+ %s\n", command);

    return system(command);
}

int get_local_id(long global_id) {
    char command[1024];
    snprintf(command, sizeof(command), "cat %s/local/%lu", db_dir, global_id);
    log_printf("+ %s\n", command);

    FILE* output = popen(command, "r");
    if (!output) {
        log_perror("lb_db: get_local_id");
        return -1;
    }

    long local_id = -1;
    fscanf(output, "%ld", &local_id);
    pclose(output);

    return local_id;
}

int get_global_id(long local_id) {
    char command[1024];
    snprintf(command, sizeof(command), "cat %s/global/%lu", db_dir, local_id);
    log_printf("+ %s\n", command);

    FILE* output = popen(command, "r");
    if (!output) {
        log_perror("lb_db: get_global_id");
        return -1;
    }

    long global_id = -1;
    fscanf(output, "%ld", &global_id);
    pclose(output);

    return global_id;
}
