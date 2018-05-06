#include <stdio.h>
#include <stdlib.h>
#include "log.h"
#include "mom.h"

#define USAGE 0
#define EXIT 1
#define CONTINUE 2

#define BUFFER_SIZE 100
#define LINE_SIZE 500

void print_usage() {
    // log_printf("And here I would put my usage message... if only I had one\n");
    log_printf("Usage:\n\
                \tp <topic> <msg>\t // Publishes msg to a topic\n\
                \tr              \t // Reads a message (blocking)\n\
                \t[s|u] <topic>  \t // Suscribe/unsubcribe to a topic\n\
                \tq              \t // Quits CLI\n");
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
    char* retrieve_topic;
    switch (cmd) {
        case 's':
            return subscribe(id, topic);
        case 'u':
            return unsubscribe(id, topic);
        case 'p':
            return publish(id, topic, payload);
        case 'r':
            retrieve(id, &retrieve_topic, &retrieve_payload);
            log_printf("Message from topic %s: %s\n", retrieve_topic, retrieve_payload);
            free(retrieve_payload);
            free(retrieve_topic);
            return 0;
    }
    return 0;
}

int parse_line(char* buffer, long id) {
    if (!is_valid_command(buffer[0])) {
        return USAGE;
    }

    // If it is not exit, keep command and continue parsing
    if (buffer[0] == 'q') return EXIT;
    char cmd = buffer[0];
    buffer++;

    // If it is not a read command, a topic is needed
    char topic[BUFFER_SIZE] = {0};
    int counter = 0;
    if (cmd != 'r') {
        // skip spaces
        if (buffer[0] != ' ') return USAGE;
        for ( ; *(buffer) == ' '; buffer++) ;
        if (is_separator(buffer[0])) return USAGE;

        while (!is_separator(*buffer) && counter < BUFFER_SIZE-1) {
            topic[counter] = *buffer;
            buffer++;
            counter++;
        }
        topic[counter] = '\0';
    }

    // If it is a publish command, a payload is needed
    char payload[BUFFER_SIZE] = {0};
    if (cmd == 'p') {
        // skip spaces
        for ( ; *(buffer) == ' '; buffer++) ;
        if (is_separator(buffer[0])) return USAGE;

        counter = 0;
        while (*(buffer) != '\n' && counter < BUFFER_SIZE-1) {
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
    log_printf("Got local id: %ld\n", id);

    int size = LINE_SIZE;
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
