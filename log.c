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
    char buffer[1024];
    int pid = getpid();

    *buffer = '\0';

    // If stdout is a tty - change color
    if (isatty(1)) {
        sprintf(buffer, "\x1b[1;38;5;%dm", ((pid % 20) * 2 + 1));
    }

    va_list args;
    va_start(args, msg);

    sprintf(buffer, "%s[%d %s] - ", buffer, pid, program_invocation_short_name);
    char aux[1024];
    vsprintf(aux, msg, args);
    va_end(args);

    sprintf(buffer, "%s%s", buffer, aux);

    // If stdout is a tty - restore color
    if (isatty(1)) {
        sprintf(buffer, "%s\x1b[39m", buffer);
    }

    printf("%s", buffer);
}

void log_perror(char* msg) {
    int old_errno = errno;
    int pid = getpid();

    printf("[%d %s] - error: ", pid, program_invocation_short_name);

    errno = old_errno;
    perror(msg);
}

#endif // _LOG_H
