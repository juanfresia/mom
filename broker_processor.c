#define _POSIX_C_SOURCE 200112L // sigaction

#include <signal.h>
#include <stdio.h>


static int quit = 0;

void graceful_quit(int sig) {
	quit = 1;
}

int main(void) {
    printf("Broker processor started!!\n");

    // Setting signal handler for graceful_quit
    struct sigaction sa = {0};
    sa.sa_handler = graceful_quit;
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);

    while(!quit) {
        printf("Broker processor says: I'm still alive!!\n");
        sleep(3);
    }

    printf("Broker processor quits\n");
    return 0;
}
