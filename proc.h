#ifndef _PROC_H
#define _PROC_H

#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define SIGTERM_ATTEMPTS 3
#define GRACEFULL_QUIT_TIME 2

int graceful_quit_or_kill(int pid, int* wstatus) {
    // Attempt to gracefully kill process
    for (int i = 0; i < SIGTERM_ATTEMPTS; i++) {
        printf("Sending SIGTERM to [%d]\n", pid);
        int r = kill(pid, SIGTERM);
        if (r < 0) return r;

        sleep(GRACEFULL_QUIT_TIME);

        int wpid = waitpid(pid, wstatus, WNOHANG);
        // Error or success
        if (wpid != 0)
            return wpid;
        // Process did not terminated properly, try again
    }

    // Process did not finished correctly sending sigkill

    printf("Sending SIGKILL to [%d]\n", pid);
    int r = kill(pid, SIGKILL);
    if (r < 0) return r;

    int wpid = waitpid(pid, wstatus, 0);
    return wpid;
}

#endif //_PROC_H
