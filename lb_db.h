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

#include "system.h"

#define ENV_LB_DATABASE_DIR "LB_DATABASE_DIR"
#define LB_DATABASE_DIR_DEFAULT "./lb_data"

char* db_dir = NULL;

/*
 * Initialize local directories where to store the id mappings.
 * If the environment variable LB_DATABASE_DIR is set, it is used,
 * otherwise the value is defaulted to LB_DATABASE_DIR_DEFAULT.
 */
int init_directories() {
    db_dir = getenv(ENV_LB_DATABASE_DIR);
    if (!db_dir) {
        db_dir = LB_DATABASE_DIR_DEFAULT;
        setenv(ENV_LB_DATABASE_DIR, db_dir, 0);
    }

    char command[1024];
    snprintf(command, sizeof(command), "mkdir -p %s %s/global %s/local", db_dir, db_dir, db_dir);
    printf("+ %s\n", command);

    int r = bash_exec(command);
    if (r < 0) {
        perror("lb_db: init_directories");
        return r;
    }
    return 0;
}

int set_local_id(long local_id, long global_id) {
    char command[1024];
    snprintf(command, sizeof(command), "echo '%lu' > %s/local/%lu", local_id, db_dir, global_id);
    printf("+ %s\n", command);

    int r = bash_exec(command);
    if (r < 0) {
        perror("lb_db: set_local_id");
        return r;
    }
    return 0;
}

int get_local_id(long global_id) {
    char command[1024];
    snprintf(command, sizeof(command), "cat %s/local/%lu", db_dir, global_id);
    printf("+ %s\n", command);

    char* output = bash_exec_output(command);
    if (!output) {
        perror("lb_db: get_local_id");
        return -1;
    }
    int local_id = -1;
    sscanf(output, "%d", &local_id);
    free(output);

    return local_id;
}
