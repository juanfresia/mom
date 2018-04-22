#ifndef SERVER_H
#define SERVER_H

#include "socket.h"

typedef enum _server_type {
	SERVER_CONCURRENT = 0,
	SERVER_ITERATIVE
} server_type;


/*
 * Signature of the connection handler function, to be called when a new
 * connection is accepted.
 */
typedef void (*handler_t) (socket_t* s);

/*
 * Starts a new server with an accepting connection on ip:port.
 * Each time a new connection starts, the callback function will be called.
 */
void run_server(char* ip, char* port, handler_t callback, server_type type);


#endif // SERVER_H
