#define _POSIX_C_SOURCE 200112L

#include <socket_factory.h>
#include <signal.h>
#include <stdbool.h>
#include <log4j.h>
#include <errno.h>
#include <socket_factory_mgr.h>
#include <bits/types/struct_timeval.h>

// global run var
volatile sig_atomic_t stop;

void sigint_handler(int sig)
{
    if (sig == SIGINT)
    {
        stop = 1;
    }
}

/*!
 * Attempt to set up a server by fetching a available sock_addr from
 * get_addr_info. On success, start the listen on the socket. All errors
 * are logged to syslog.
 *
 * @param port_addr String port to attempt to listen on
 * @param payload_size size_t of the default payload to listen for
 * @param ipv6_only bool indicating if a IPv6 is choosen if it should only
 * accept IPv6 or if it should map to IPv4 as well.
 * @return socket_factory_t
 */
socket_factory_t * tcp_server_setup(const char *port_addr,
                                   char *root,
                                   size_t payload_size,
                                   bool ipv6_only)
{
    // create the factory structure
    socket_factory_t * factory = factory_init(payload_size);
    factory->root = root;

    // set a log_msg string
    int log_msg_len = 2048;
    char * log_msg = calloc(1, sizeof(char) * log_msg_len);

    // server vars
    int socket_fd = 0;
    int str_err = 0;

    // create structure for network translation
    struct addrinfo * results;
    struct addrinfo * network_record;

    // create hints for fetching the correct socket structure
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;        // use IPv6 or IPv4
    hints.ai_socktype = SOCK_STREAM;    // use tcp stream
    hints.ai_flags = AI_PASSIVE;        // use my IP
    hints.ai_protocol = IPPROTO_TCP;    // use TCP proto

    // attempt to fetch a result
    if ((getaddrinfo(NULL, port_addr, &hints, &results)) != 0)
    {
        perror("Failed to translate server socket");
        factory_destroy(factory);
        return NULL;
    }

    // keep track of the root record to free them later or else you will get
    // a leak from freein gthe link list
    struct addrinfo * root_record = results;

    // iterate over the link list of resolution network records
    for (network_record = results; network_record != NULL;
                                    network_record = network_record->ai_next)
    {
        // attempt to create a socket out of the record. If it fails, fetch
        // the next one
        socket_fd = socket(
            network_record->ai_family,
            network_record->ai_socktype,
            network_record->ai_protocol
        );
        if (-1 == socket_fd)
        {
            continue;
        }

        // Set the socket to reuse its address. This is useful for
        // continuously listening for new connections
        int enable = 1;
        if ((network_record->ai_family == AF_INET6) && (ipv6_only))
        {
            // If ipv6 is used, ensure that no network mapping is used
            str_err = setsockopt(
                socket_fd,
                IPPROTO_IPV6,
                IPV6_V6ONLY,
                &enable,
                sizeof(int)
            );
            if (-1 == str_err)
            {
                clear_char(log_msg, log_msg_len);
                sprintf(log_msg, "setsockopt error: %s", strerror(errno));
                log_error(log_msg);
                close(socket_fd);
                continue;
            }

        }
        str_err = setsockopt(
            socket_fd,
            SOL_SOCKET,
            SO_REUSEADDR,
            &enable,
            sizeof(int)
        );
        if (-1 == str_err)
        {
            clear_char(log_msg, log_msg_len);
            sprintf(log_msg, "setsockopt error on port %s: %s",
                    port_addr, strerror(errno));
            log_error(log_msg);
            close(socket_fd);
            continue;
        }

        // attempt to bind with the socket to ensure that it works
        if (0 == (
            bind(
            socket_fd,
            network_record->ai_addr,
            network_record->ai_addrlen))
            )
        {
            break; // if we get here, then the socket is functioning
        }
        else
        {
            // if we get here then we know that the socket was created
            // successfully, but we were not able to bind to the address
            // provided. So close it and try the next one
            close(socket_fd);
            clear_char(log_msg, log_msg_len);
            sprintf(log_msg, "bind error: %s", strerror(errno));
            log_error(log_msg);
            continue;
        }
    }

    // if network_record is still NULL then we know that none of the
    // translations returned
    if (NULL == network_record)
    {

        clear_char(log_msg, log_msg_len);
        sprintf(log_msg, "None of the network translations were successful\n");
        log_error(log_msg);
        free(log_msg);
        freeaddrinfo(root_record);
        factory_destroy(factory);
        return NULL;
    }
    // set socket to listen
    factory->socket = socket_fd;
    if (-1 == listen(factory->socket, BACKLOG))
    {
        close(socket_fd);
        clear_char(log_msg, log_msg_len);
        sprintf(log_msg, "LISTEN %s", strerror(errno));
        log_error(log_msg);
        freeaddrinfo(root_record);
        factory_destroy(factory);
        return NULL;
    }

    // ensure that any children of the listener do not need to report back
    struct sigaction ignorer = {0};
    ignorer.sa_handler = SIG_IGN;
    str_err = sigaction(SIGCHLD, &ignorer, NULL);
    if(0 > str_err)
    {
        close(socket_fd);
        clear_char(log_msg, log_msg_len);
        sprintf(log_msg, "sigaction configuration: %s", strerror(errno));
        log_error(log_msg);
        freeaddrinfo(root_record);
        factory_destroy(factory);
        return NULL;
    }

    // allocate the space for the self config
    factory->self_config = calloc(1, sizeof(* factory->self_config));


    // copy the working network config
    memcpy(factory->self_config, network_record->ai_addr,
           network_record->ai_addrlen);

    // fill in address information
    factory->ip_addr = get_ip(factory->self_config);
    factory->port = get_port(factory->self_config);


    // log message
    sprintf(log_msg, "Running on http://%s:%s",
            factory->ip_addr, factory->port);
    log_info(log_msg);

    // free the link list and log
    freeaddrinfo(root_record);
    free(log_msg);

    return factory;
}

/*!
 * Free up the socket factory structure, this includes the structure created
 * by the getaddrinfo function
 *
 * @param factory socket_factory_t pointer
 */
void factory_destroy(socket_factory_t * factory)
{
    // close any fd if it is greater than -1
    if (-1 < factory->socket)
    {
        close(factory->socket);
    }

    // free remote if available
    if (NULL != factory->remote)
    {
        free_source_client(factory);
        free(factory->remote->remote_config);
        free(factory->remote);
    }

    // Free memory for factory
    if (NULL != factory->payload)
    {
        free(factory->payload);
    }

    free(factory->self_config);
    free(factory->ip_addr);
    free(factory->port);
    if (NULL != factory->root)
    {
        free(factory->root);
    }
    free(factory);
}


/*!
 * Handle listening on the socket specified. Any connections will then be forked
 * and passed to the callback function
 * @param factory factory pointer
 * @param callback callback function the forked child calls
 * @param connection_count Amount of connections to listen to before dying.
 * Giving a value of 0 sets it to listen for ever
 */
void tcp_serve(socket_factory_t *factory,
                         void (*callback)(socket_factory_t *),
                         int connection_count)
{


    // set up sigint handler
    struct sigaction signal_action;
    memset(&signal_action, 0, sizeof(signal_action));
    signal_action.sa_handler = sigint_handler;
    if (-1 == (sigaction(SIGINT, &signal_action, NULL)))
    {
        perror("sighandler");
        return;
    }

    // set flag indicating if the server should run forever or for x amount
    // of connections specified by the paramter
    int connections = 0;
    bool run_forever = false;
    if (0 == connection_count)
    {
        run_forever = true;
        connection_count = 2;
    }
    while ((connections < connection_count) && (!stop))
    {
        char log_msg[2048];
        int client_socket = 0;
        client_socket = accept(
            factory->socket,
            ((struct sockaddr *)factory->remote->remote_config),
                &factory->remote->remote_sz
            );

        // if connection was an error log it
        if (0 > client_socket)
        {
            sprintf(log_msg, "ACCEPT: %s", strerror(errno));
            log_error(log_msg);
            close(client_socket);
        }
        // if we are here, then the connection was successful
        else
        {
            // if run is limited, then update the connection counter
            if (!run_forever)
            {
                connections++;
            }


            // populate the remote record and log the information
            populate_source_info(factory);
            sprintf(log_msg, "Received connection from %s:%s",
                    factory->remote->ip_addr, factory->remote->port);
            log_info(log_msg);


            // fork to let the connection be handle by the callback
            int pid = fork();

            // child should close the server socket and call the callback
            // function to handle the connection. Then cleanly exit
            if (0 == pid)
            {

                close(factory->socket);
                factory->socket = client_socket;
                callback(factory);
                sprintf(log_msg, "Connection complete for %s:%s",
                        factory->remote->ip_addr, factory->remote->port);
                log_info(log_msg);
                factory_destroy(factory);
                _Exit(3);
            }

            close(client_socket);
            reset_storage(factory);
        }
    }
}

/*!
 * Receive HTTP `response` from a client. The recv is terminated when the
 * rfc2616 CRLF CRLF (\r\n\r\n) is detected. The socket will also timeout
 * after 10 seconds if no data is received that terminates the recv
 *
 * @param factory factory pointer
 * @return -1 for error/complete or 0 for received payload
 */
int8_t recv_payload(socket_factory_t *factory)
{

    char log_msg[2048];
    sprintf(log_msg, "Ready to receive request from %s:%s",
            factory->remote->ip_addr, factory->remote->port);
    log_info(log_msg);

    // create a destination buffer with the max get request payload size
    char recv_payload[MAX_GET_REQ + 1];
    clear_char(recv_payload, MAX_GET_REQ + 1);
    clear_buffer(factory);
    ssize_t bytes = 0;
    bool keep_reading = true;

    while (keep_reading)
    {
        bytes = recv(factory->socket, factory->payload,
                     factory->payload_size - 1, 0);
        if (-1 == bytes)
        {
            if ((EAGAIN == errno) || (EWOULDBLOCK == errno))
            {
                sprintf(log_msg, "Connection timed out for %s:%s",
                        factory->remote->ip_addr, factory->remote->port);
            }
            else{
                sprintf(log_msg, "RECV: %s", strerror(errno));
            }
            log_error(log_msg);
            return -1;
        }
        // if we are here then the client terminated the connection
        else if (0 == bytes)
        {
            return -1;
        }

        // update the payload
        factory->payload[bytes] = '\0';
        strncat(recv_payload, factory->payload, bytes);

        // get payload length to inspect it
        size_t payload_length = strlen(recv_payload);

        // check if we are done reading
        if (4 < payload_length)
        {
            if (recv_payload[payload_length - 1] == '\n')
            {
                if (
                    (recv_payload[payload_length - 2] == '\r') ||
                    (recv_payload[payload_length - 3] == '\n') ||
                    (recv_payload[payload_length - 4] == '\r')
                    )
                {
                    keep_reading = false;
                }
            }
        }
    }

    size_t payload_len = strlen(recv_payload);

    sprintf(log_msg, "Received %zu bytes from %s:%s", payload_len,
            factory->remote->ip_addr, factory->remote->port);
    log_info(log_msg);

    // check if buffer requirements have changed
    if (payload_len > factory->payload_size)
    {
        free(factory->payload);
        factory->payload_size = payload_len;
        factory->payload = malloc(sizeof(char) * factory->payload_size + 1);
    }
    clear_buffer(factory);
    strncpy(factory->payload, recv_payload, payload_len + 1);
    return 0;
}

/*!
 * Send a string payload to the client
 * @param factory factory pointer
 * @param payload string contianing the data to send to the client
 * @return -1 for error 0 for success
 */
int8_t send_payload(socket_factory_t *factory, char * payload)
{
    char log_msg[1024];

    ssize_t bytes = send(
        factory->socket,
        payload,
        strlen(payload),
        MSG_NOSIGNAL
        );

    if (-1 == bytes)
    {
        // log errors if any
        sprintf(log_msg, "SEND: %s", strerror(errno));
        log_error(log_msg);
        return -1;
    }

    sprintf(log_msg, "Sent %zd bytes to %s:%s", bytes,
            factory->remote->ip_addr, factory->remote->port);
    log_info(log_msg);
    return 0;
}

/*!
 * Set the client socket to timeout by the specified time
 * @param factory factory pointer
 * @return -1 for failure 0 for success
 */
int8_t set_client_timeout(socket_factory_t * factory)
{

    char log_msg[1024];
    struct timeval tv;
    tv.tv_usec = 0;
    tv.tv_sec = TIMEOUT;

    int str_err = setsockopt(
        factory->socket,
        SOL_SOCKET,
        SO_RCVTIMEO,
        (const char*)&tv,
        sizeof(tv)
    );

    if (-1 == str_err)
    {
        clear_char(log_msg, 1024);
        sprintf(log_msg, "Unable to modify socket %s:%s: %s",
                factory->remote->ip_addr,
                factory->remote->port, strerror(errno));
        log_error(log_msg);
        return -1;
    }
    return 0;
}
