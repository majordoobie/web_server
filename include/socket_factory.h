#ifndef ASHTI_SRC_SOCKET_FACTORY_H_
#define ASHTI_SRC_SOCKET_FACTORY_H_
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <netdb.h>
#include <syslog.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>

enum {BUFF_SIZE = 512, BACKLOG = 512, MAX_GET_REQ = 2048, TIMEOUT = 10};

typedef struct {
    char * ip_addr;                     // String IP address of remote
    char * port;                        // String port of remote
    struct sockaddr_storage * remote_config;
    socklen_t remote_sz;
} source_address_t;

typedef struct {
    char * root;
    char * ip_addr;           // String IP address
    char * port;              // String port

    int socket;                     // The socket for communication
    char * payload;                 // Buffer for server/remote communication
    size_t payload_size;            // size of the payload

    struct sockaddr_storage * self_config;
    socklen_t self_config_sz;

    source_address_t * remote;      // struct to remote from the receiving end
}socket_factory_t;


void factory_destroy(socket_factory_t * factory);

// tcp set up
socket_factory_t *tcp_server_setup(const char *port_addr,
                                   char *root,
                                   size_t payload_size,
                                   bool ipv6_only);

// listen on the socket
void tcp_serve(socket_factory_t *factory,
                         void (*callback)(socket_factory_t *),
                         int connection_count);

// receive data from client socket
int8_t recv_payload(socket_factory_t *factory);
int8_t send_payload(socket_factory_t *factory, char * payload);
int8_t set_client_timeout(socket_factory_t * factory);

#endif //ASHTI_SRC_SOCKET_FACTORY_H_
