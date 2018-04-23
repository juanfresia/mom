#ifndef _SYSTEM_H
#define _SYSTEM_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_OUTPUT 1024

int bash_exec(const char* cmd) {
    return system(cmd);
}

char* bash_exec_output(const char *cmd) {
    int pipes[2] = {0};
    int r = pipe(pipes);
    if (r < 0) {
        return 0;
    }

    int pid = fork();
    if (pid < 0) {
        close(pipes[0]);
        close(pipes[1]);
        return NULL;
    }

    // Child
    if (pid == 0) {
        close(pipes[0]);
        // Override stdout
        dup2(pipes[1], 1);
        execl("/bin/sh", "sh", "-c", cmd, 0);   // system(command);
        _exit(-1);
    }

    close(pipes[1]);
    char *output = malloc(sizeof(char) * MAX_OUTPUT);
    *output = '\0';
    int bytes_read = 0;
    int tmp = 0;
    do {
        tmp = read(pipes[0], output+bytes_read, sizeof(output)-bytes_read);
        if (tmp < 0) {
            _exit(-1);
        }
        bytes_read += tmp;
    } while(tmp > 0);

    output[bytes_read] = '\0';

    waitpid(pid, 0, 0);
    close(pipes[0]);

    return output;
}

#endif // _SYSTEM_H
