#include <stdio.h>
#include "log.h"
#include "mom.h"

void print_usage() {
    log_printf("And here I would put my usage message... if only I had one\n");
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

int parse_line(char* buffer, long id) {
    if (!is_valid_command(buffer[0])) {
        return 0;
    }
    // Is a valid command

    if (buffer[0] == 'q') return 1;
    char cmd = buffer[0];
    buffer++;

    // skip spaces
    if (buffer[0] != ' ') return 0;
    for ( ; *(buffer) == ' '; buffer++) ;
    if (is_separator(buffer[0])) return 0;

    char topic[100] = {0};
    int counter = 0;
    while (!is_separator(*(buffer))) {
        topic[counter] = *buffer;
        buffer++;
        counter++;
    }
    topic[counter] = '\0';

    char payload[100];
    if (cmd == 'p') {
        // skip spaces
        for ( ; *(buffer) == ' '; buffer++) ;
        if (is_separator(buffer[0])) return 0;

        counter = 0;
        while (!is_separator(*(buffer))) {
            payload[counter] = *buffer;
            buffer++;
            counter++;
        }
        payload[counter] = '\0';

    }

    switch (cmd) {
        case 's':
            return subscribe(id, topic);
        case 'p':
            return publish(id, topic, payload);
        case 'u':
            return unsubscribe(id, topic);
    }
    return 0;
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
    int quit = 0;

    while(!quit) {
        if (!fgets(buffer, size, stdin)) break;

        quit = parse_line(buffer, id);
    }

    //
    // int r = subscribe(id, "topic1");
    // if (r < 0) {
    //     perror("client: register");
    // }
    //
    // r = subscribe(id, "topic1/topic2");
    // if (r < 0) {
    //     perror("client: register");
    // }
    //
    // r = publish(id, "placeholder", "Esto es un mensaje de prueba");
    // if (r < 0) {
    //     perror("client: register");
    // }
    //
    unregister(id);
    log_printf("Shutting down client\n");
    return 0;
}
