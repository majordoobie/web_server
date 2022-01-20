#define _GNU_SOURCE
#include <socket_factory_mgr.h>
#include <linux/limits.h>
#include <dirent.h>

char * get_full_path(const char * path)
{
    // attempt to resolve path provided
    char resolved_path[PATH_MAX];
    realpath(path, resolved_path);

    // ensure proper permissions are set to read path
    DIR* dir = opendir(resolved_path);
    if (NULL == dir)
    {
        perror("web server root dir");
        closedir(dir);
        return NULL;
    }
    closedir(dir);

    // malloc the string for the path
    size_t len = (strlen(resolved_path) + 1);
    char * new_path = calloc(1, sizeof(char) * len);
    strncpy(new_path, resolved_path, len);
    return new_path;
}

/*!
 * Create the socket_factory structure
 *
 * @param payload_size default size for the payload in the structure
 * @return pointer to socket_factory_t structure
 */
socket_factory_t *factory_init(size_t payload_size)
{
    // calloc the overall structure
    socket_factory_t * new_factory = calloc(1, sizeof(* new_factory));
    if (NULL == new_factory)
    {
        fprintf(stderr, "Could not allocated memory\n");
        abort();
    }


    // create the payload structure and set it to NULL
    new_factory->payload_size = payload_size;
    new_factory->payload = calloc(1, sizeof(char) * payload_size);
    if (NULL == new_factory->payload)
    {
        fprintf(stderr, "Could not allocated memory\n");
        abort();
    }

    // create the remote configuration
    new_factory->remote = calloc(1, sizeof(* new_factory->remote));
    if (NULL == new_factory->remote)
    {
        fprintf(stderr, "Could not allocated memory\n");
        abort();
    }

    // set remote values to NULL
    new_factory->remote->remote_config = calloc(
        1,
        sizeof(* new_factory->remote->remote_config)
    );
    new_factory->remote->remote_sz = sizeof(*new_factory->remote->remote_config);

    new_factory->remote->port = NULL;
    new_factory->remote->ip_addr = NULL;

    new_factory->ip_addr = NULL;
    new_factory->port = NULL;

    new_factory->socket = -1;
    new_factory->self_config = NULL;

    return new_factory;
}

/*!
 * Free the remote structure inside the factory structure and set their values
 * to NULL for reuse.
 *
 * @param factory Factory pointer
 */
void free_source_client(socket_factory_t * factory)
{
    if (NULL != factory->remote->ip_addr)
    {
        free(factory->remote->ip_addr);
        factory->remote->ip_addr = NULL;
    }

    if (NULL != factory->remote->port)
    {
        free(factory->remote->port);
        factory->remote->port = NULL;
    }
}

/*!
 * Helper function to reset the remote config
 */
void reset_storage(socket_factory_t * factory)
{
    memset(factory->remote->remote_config, 0, factory->remote->remote_sz);
    clear_buffer(factory);
    free_source_client(factory);
}

/*!
 * Helper function to clear the payload buffer
 */
void clear_buffer(socket_factory_t * factory)
{
    // Zero out payload for clean data read
    clear_char(factory->payload, factory->payload_size);
}

/*!
 * Helper function to clear any buffer
 * @param buffer
 * @param buffer_sz
 */
void clear_char(char * buffer, size_t buffer_sz)
{
    memset(buffer, 0, buffer_sz);
}

/*!
 * Helper function to convert the sockaddr_storage structure to a IP string
 * @param config pointer to a sockadd_storage
 * @return
 */
char * get_ip(struct sockaddr_storage * config)
{
    char addr[INET6_ADDRSTRLEN];
    struct sockaddr * conf = (struct sockaddr *)config;

    switch(conf->sa_family)
    {
        case AF_INET:
        {
            struct sockaddr_in * in4 = (struct sockaddr_in *)config;
            inet_ntop(
                in4->sin_family,
                &in4->sin_addr,
                addr,
                sizeof(addr)
            );
            break;
        }
        case AF_INET6:
        {
            struct sockaddr_in6 * in6 = (struct sockaddr_in6 *)config;
            inet_ntop(
                in6->sin6_family,
                &in6->sin6_addr,
                addr,
                sizeof(addr)
            );
            break;
        }
    }

    // save server socket values
    size_t size = strlen(addr) + 1;
    char * addr_str = malloc(sizeof(char) * size);
    strncpy(addr_str, addr, size);
    return addr_str;
}

/*!
 * Helper function to convert the sockaddr_storage structure to a port string
 * @param config pointer to a sockadd_storage
 * @return
 */
char * get_port(struct sockaddr_storage * config)
{
    unsigned short port = 0;
    struct sockaddr * conf = (struct sockaddr *)config;

    switch(conf->sa_family)
    {
        case AF_INET:
        {
            struct sockaddr_in * in4 = (struct sockaddr_in *)config;
            port = ntohs(in4->sin_port);
            break;
        }
        case AF_INET6:
        {
            struct sockaddr_in6 * in6 = (struct sockaddr_in6 *)config;
            port = ntohs(in6->sin6_port);
            break;
        }
    }

    // copy the port
    char port_string[65600];
    sprintf(port_string, "%d", port);
    size_t size = strlen(port_string) + 1;
    char * port_str = calloc(1, (size * sizeof(char)));
    strncpy(port_str, port_string, size);
    return port_str;
}


/*!
 * Helper function to populate the factory remote config with the string
 * representation of the storage
 */
void populate_source_info(socket_factory_t *factory)
{
    free_source_client(factory);
    factory->remote->ip_addr = get_ip(factory->remote->remote_config);
    factory->remote->port = get_port(factory->remote->remote_config);
}
