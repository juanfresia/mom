#ifndef _LOG_H
#define _LOG_H

#define _GNU_SOURCE
#include "log.h"


#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

extern char *program_invocation_short_name;

void log_printf(char* msg, ... ) {
    setbuf(stdout, NULL);
    int pid = getpid();
    va_list args;
    va_start(args, msg);

    printf("[%d %s] - ", pid, program_invocation_short_name);
    vprintf(msg, args);
    va_end(args);
}

void log_perror(char* msg) {
    int old_errno = errno;
    int pid = getpid();

    printf("[%d %s] - error: ", pid, program_invocation_short_name);

    errno = old_errno;
    perror(msg);
}

#endif // _LOG_H
