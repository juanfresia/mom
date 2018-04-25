#include <stdio.h>
#include "lb.h"
#include "server.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT "12345"

void handler(socket_t* s) {
    printf("A connection has arrived\n");

    struct lb_msg_t msg = {0};
    for (int i = 0; i < 10; i++) {
        int r = SOCK_RECV(s, struct lb_msg_t, msg);
        if (r < 0) {
            perror("server_handler: sock_recv");
        }
        printf("I received a message! [%lu, %s]\n", msg.global_id, MSG_TYPE_TO_STRING(msg.type));

        msg.type = MSG_ACK_OK;

        r = SOCK_SEND(s, struct lb_msg_t, msg);
        if (r < 0) {
            perror("server_handler: sock_send");
        }
    }
}

int main (int argc, char* argv[]) {
    // Init broker processor queues

    // Start broker processor

    // Run server accepting connections
    run_server(SERVER_IP, SERVER_PORT, handler, SERVER_CONCURRENT);

    // Server gracefully exited
    // Gracefully kill children

    // Free resources
    return 0;
}
