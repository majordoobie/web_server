#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <getopt.h>
#include <linux/limits.h>
#include <dirent.h>

#include <c_subprocess.h>
#include <socket_factory.h>
#include <http_handler.h>
#include "file_fetcher.h"
#include "log4j.h"
#include <socket_factory_mgr.h>

typedef struct
{
    char * file_path;
    int connections;
} server_struct_t;


static void client_loop(socket_factory_t * factory);
static int validate_int(char *argument);
static void print_error();
static server_struct_t * parse_args(int argc, char ** argv);


int main(int argc, char ** argv)
{
    // if arguments were not properly parsed then exit
    server_struct_t * server = parse_args(argc, argv);
    if (NULL == server)
    {
        return -1;
    }

    // extract the struct and free it
    char * root_path = server->file_path;
    int connection = server->connections;
    free(server);

    uid_t port = getuid();
    if (1024 > port)
    {
        port += 2000;
    }
    char port_str[10];
    sprintf(port_str, "%d", port);


    // set up listener
    socket_factory_t * factory = tcp_server_setup(
        port_str,
        root_path,
        512,
        false
        );

    if (NULL == factory)
    {
        return -1;
    }

    tcp_serve(
        factory,
        client_loop,
        connection);

    // clean up and exit
    factory_destroy(factory);
    return 0;
}


/*!
 * Read the static resource requested and return the data as a string array
 *
 * @param server file_return_t structure
 * @return String payload
 */
static char *get_static_resource(file_return_t * server)
{
    FILE * html_data = fopen(server->file_path, "r");
    if (NULL == html_data)
    {
        perror("html_data");
        return NULL;
    }

    // allocate the space for the response payload
    char * payload = calloc(1, (sizeof(char) * server->file_size) + 1);

    // read the html file to extract the contents
    fread(payload, server->file_size, 1, html_data);
    fclose(html_data);
    payload[server->file_size] = '\0';

    // return the payload
    return payload;

}
static file_return_t *init_server_struct()
{
   file_return_t * server = calloc(1, sizeof(* server));
   server->status = ok;
   server->source_type = www;
   server->file_path = NULL;
   server->root_dir = NULL;
   server->target_file = NULL;
   server->err_msg = NULL;
   server->error_dir = NULL;
   return server;
}

static int8_t send_payload_t(socket_factory_t * factory, file_return_t *
                                server, char * payload)
{
    char * http_payload = http_response(server, payload);

    int8_t status = send_payload(factory, http_payload);

    free(payload);
    free(http_payload);
    return status;
}

static void reset_ret(file_return_t *server)
{
    server->status = ok;
    server->source_type = www;
    server->file_size = 0;
    if (NULL != server->file_path)
    {
        free(server->file_path);
        server->file_path = NULL;
    }

    if (NULL != server->target_file)
    {
        free(server->target_file);
        server->target_file = NULL;
    }

    if (NULL != server->err_msg)
    {
        free(server->err_msg);
        server->err_msg = NULL;
    }
}
static void client_loop(socket_factory_t * factory)
{
    // create the server structure and assign the root directory
    file_return_t * server = init_server_struct();
    server->root_dir = factory->root;
    factory->root = NULL;
    server->error_dir = calloc(1, strlen(server->root_dir) + 7);
    strncpy(server->error_dir, server->root_dir, strlen(server->root_dir));
    strncat(server->error_dir, "/error", 7);

    // modify the client socket
    set_client_timeout(factory);

    // begin to perform client loop
    int8_t res = recv_payload(factory);
    while(res == 0)
    {
        reset_ret(server);

        // var for response
        char * response_payload = NULL;

        // parse the get request
        http_parse(server, factory->payload);
        char log[2048];
        sprintf(log, "URI Request: %s", server->target_file);
        log_info(log);

        // if we did not receive a 200 from parser, return error to user
        if (ok != server->status)
        {
            response_payload = read_html_err(server);
            res = send_payload_t(factory, server, response_payload);
            continue;
        }

        // attempt to fetch the resource requested
        file_finder(server);
        if (ok != server->status)
        {
            response_payload = read_html_err(server);
            res = send_payload_t(factory, server, response_payload);
            continue;
        }

        if (www == server->source_type)
        {
            response_payload = get_static_resource(server);
            res = send_payload_t(factory, server, response_payload);
            continue;
        }
        else
        {
            // if cgi script and not executable, error
            if (!server->execute)
            {
                response_payload = read_html_err(server);
                res = send_payload_t(factory, server, response_payload);
                continue;
            }

            // run the cgi program specified
            response_payload = c_subprocess(server, NULL);
            if (ok != server->status)
            {
                free(response_payload);
                response_payload = read_html_err(server);
                res = send_payload_t(factory, server, response_payload);
                continue;
            }
            else
            {
                send_payload(factory, response_payload);
                free(response_payload);
            }
        }

        // get next payload
        res = recv_payload(factory);
    }
    close(factory->socket);
    f_ret_destroy(server);

}

/*!
 * Print out a helper message when an error occurs
 */
static void print_error()
{
    fprintf(stderr,
            "Invalid argument supplied. The server must receive a mandatory "
            "file path to its root folder. Additionally, a -c option may be "
            "supplied to modify for how many connections it should listen to "
            "before it errors. A value of 0 will listen for ever.\n\n"
            "Examples:\n\t./ashti /path/to/www_root\n\t./ashti www_root -c "
            "4\n");
}

/*!
 * Private function to validate the arguments passed in.
 *
 * @param argc argument count
 * @param argv arguments
 * @return NULL if error otherwise a server_structure_t
 */
static server_struct_t * parse_args(int argc, char ** argv)
{
    int getopt_res = 0;
    bool connection_used = false;
    int connection = 0;

    while ((getopt_res = getopt(argc, argv, "c:")) != -1)
    {
        if ('c' == getopt_res)
        // set the listen port. This hidden option is only used for workers
        {
            if (!connection_used)
            {
                int int_val = validate_int(optarg);
                if (int_val < 0)
                {
                    continue;
                }

                // set port used to true
                connection_used = true;
                connection = int_val;
            }
        }

        else if ('?' == getopt_res)
        {
            print_error();
            return NULL;
        }
    }

    // If there is anything else to parse, return an error
    if (optind != (argc - 1))
    {
        print_error();
        return NULL;
    }

    // attempt to resolve path provided
    char resolved_path[PATH_MAX];
    realpath(argv[optind], resolved_path);

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
    server_struct_t * server = calloc(1, sizeof(* server));
    server->file_path = get_full_path(argv[optind]);
    server->connections = connection;
    return server;
}

static int validate_int(char *argument)
{
    /**
 * @brief - A private function to arg_parser.c. This function is used to
 * validate that the arguments of the s, e, and t options are valid integers.
 * The reference used to handle error responses from strtol included at the
 * top of the file.
 *
 * @param optarg - The option argument to be validated.
 * @return - The option argument converted to an integer value, or a -1 if an
 * error has occurred.
 */
    char * trash = NULL;
    int num = 0;
    errno = 0;

    num = strtol(argument, &trash, 10);

    if(argument == trash)
    {
        fprintf(stderr, "No value provided for switch\n");
        return -1;
    }
    else if((ERANGE == errno) && (INT32_MIN == num))
    {
        fprintf(stderr, "Underflow occurred for switch value\n");
        return -1;
    }
    else if((ERANGE == errno) && (INT32_MAX == num))
    {
        fprintf(stderr, "Overflow occurred for switch value\n");
        return -1;
    }
    else if(EINVAL == errno)
    {
        fprintf(stderr, "Unsupported values found within switch\n");
        return -1;
    }
    else if((0 != errno) && (0 == num))
    {
        fprintf(stderr, "Unexpected error handling switch value\n");
        return -1;
    }
    else if((0 == errno) && argument && (0 != *trash))
    {
        fprintf(stderr, "%d%s invalid switch argument\n", num, trash);
        return -1;
    }

    return num;
}

