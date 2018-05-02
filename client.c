#include <stdio.h>
#include <stdlib.h>
#include "log.h"
#include "mom.h"

#define USAGE 0
#define EXIT 1
#define CONTINUE 2

void print_usage() {
    // log_printf("And here I would put my usage message... if only I had one\n");
    log_printf("Wrong command\n");
}

int is_valid_command(char c) {
    switch (c) {
        case 's':   // subscribe
        case 'u':   // unsubscribe
        case 'p':   // publish
        case 'r':   // retrieve
        case 'q':   // quit
            return 1;
        default:
            return 0;
    }
}

int is_separator(char c) {
    switch (c) {
        case ' ':
        case '\n':
        case '\0':
            return 1;
        default:
            return 0;
    }
}

int execute_cmd(char cmd, long id, char* topic, char* payload) {
    log_printf("Going to execute %c, with [id: %ld, topic: %s, payload: %s]\n", cmd, id, topic, payload);
    char* retrieve_payload;
    switch (cmd) {
        case 's':
            return subscribe(id, topic);
        case 'u':
            return unsubscribe(id, topic);
        case 'p':
            return publish(id, topic, payload);
        case 'r':
            retrieve(id, topic, &retrieve_payload);
            log_printf("Received %s\n", retrieve_payload);
            free(retrieve_payload);
            return 0;
    }
    return 0;
}

int parse_line(char* buffer, long id) {
    if (!is_valid_command(buffer[0])) {
        return USAGE;
    }
    // Is a valid command

    if (buffer[0] == 'q') return EXIT;
    char cmd = buffer[0];
    buffer++;

    // skip spaces
    if (buffer[0] != ' ') return USAGE;
    for ( ; *(buffer) == ' '; buffer++) ;
    if (is_separator(buffer[0])) return USAGE;

    char topic[100] = {0};
    int counter = 0;
    while (!is_separator(*(buffer))) {
        topic[counter] = *buffer;
        buffer++;
        counter++;
    }
    topic[counter] = '\0';

    char payload[100] = {};
    if (cmd == 'p') {
        // skip spaces
        for ( ; *(buffer) == ' '; buffer++) ;
        if (is_separator(buffer[0])) return USAGE;

        counter = 0;
        while (!is_separator(*(buffer))) {
            payload[counter] = *buffer;
            buffer++;
            counter++;
        }
        payload[counter] = '\0';
    }

    execute_cmd(cmd, id, topic, payload);
    return CONTINUE;
}

// Local broker deamon
int main(void) {
    log_printf("Starting client\n");

    long id = register_client();
    if (id < 0) {
        log_perror("register");
    }

    int size = 200;
    char buffer[size];
    int quit = CONTINUE;

    while(quit != EXIT) {
        printf("> ");
        if (!fgets(buffer, size, stdin)) break;

        quit = parse_line(buffer, id);
        if (quit == USAGE) {
            print_usage();
        }
    }

    unregister(id);
    log_printf("Shutting down client\n");
    return 0;
}
