#include <http_handler.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

/**
 * @brief - 
 *      Parse given HTTP request string.
 *      HTTP URI request format is space delimited, with the method being 
 *          the first argument, and path being the second argument.
 *      The method is verified as GET, if not return Bad Method <method>.
 *      The path is verified to include a / at the beginning, return NULL 
 *          if it does not.
 *      The return is NULL if nothing follows GET.
 *      If the path is "/", then it is resolved to "/index.html", per bonus
 *          feature.
 *
 * @param src_str - HTTP Request string received to be parsed.
 *
 * @return file_name - Returns the parsed path from the request.
 */
void http_parse(file_return_t * f_info, char * src_str)
{
    char * file_name = NULL;
    size_t fn_size = 0;
    size_t mt_size = 0;
    char * temp = NULL;
    const char index[] = "/index.html";
    const char delim[2] = " ";
    const char bm_error[] = "405 Method Not Allowed - ";
    const char br_error[] = "400 Bad Request";

    //Tokenize string for method
    temp = strtok(src_str, delim);

    //Check if sent str was empty
    if(NULL != temp)
    {
        //Verify method was GET and not POST/PUT/etc.
        if(0 == strncmp(temp, "GET", 3))
        {
            //Tokenize string again for path argument of GET request
            temp = strtok(NULL, delim);

            //Verify first letter of path is /, invalid if not
            if((NULL != temp) && (47 == temp[0]))
            {
                //Verify if entire path is /, resolve to index if true for
                //bonus feature
                //Otherwise build path str to return
                if(0 == strcmp(temp, "/"))
                {
                    fn_size = strlen(index);
                    file_name = calloc(fn_size + 1, sizeof(char));
                    strncpy(file_name, index, fn_size);
                    f_info->target_file = file_name;
                }
                else
                {
                    fn_size = strlen(temp);
                    file_name = calloc(fn_size + 1, sizeof(char));
                    strncpy(file_name, temp, fn_size);
                    f_info->target_file = file_name;
                }
            }
            else
            {
                f_info->target_file = NULL;
                f_info->status = b_request;
                fn_size = strlen(br_error);
                file_name = calloc(fn_size + 1, sizeof(char));
                strncpy(file_name, br_error, fn_size);
                f_info->err_msg = file_name;
            }
        }
        else
        {
            //If an invalid method was used, return Bad Method <method>
            //as error message
            fn_size = strlen(bm_error);
            mt_size = strlen(temp);
            file_name = calloc(fn_size + mt_size + 1, sizeof(char));
            strncpy(file_name, bm_error, fn_size);
            strncpy(file_name + fn_size, temp, mt_size);
            f_info->target_file = NULL;
            f_info->err_msg = file_name;
            f_info->status = b_method;
        }
    }
}

/**
 * @brief - Return formatted date time string for HTTP response per RFC
 *
 * @return - Return string to match HTTP header RFC 822, updated by RFC 1123
 *              for date/time.
 */
char * http_datetime(void)
{
    int bufsize = 100;
    time_t raw;
    struct tm * tm_info = NULL;
    char * ret_time = calloc(100, sizeof(char));
    const char time_fmt[] = "%a, %d %b %Y %H:%M:%S %Z";

    time(&raw);

    //Grab time info for GMT
    tm_info = gmtime(&raw);

    //Build return time string in RFC specified format
    strftime(ret_time, bufsize - 1, time_fmt, tm_info);

    return ret_time;
}

/**
 * @brief - Build response string to send to client
 *
 * @param f_info - Structure containing requested file information
 * @param code_msg - Message from execution or read file to append to ret str
 * @return - Return complete response message to be sent to client
 */
char * http_response(file_return_t * f_info, char * code_msg)
{
    size_t total_size = strlen(code_msg) + 2048;
    char * res_str = calloc(total_size, sizeof(char));
    char * date = http_datetime();
    char st_line[1000];

    memset(st_line, 0,  sizeof(st_line));
    strcpy(st_line, "HTTP/1.1 ");

    //Add status code into HTTP status line
    if(200 == f_info->status)
    {
        strcat(st_line, "200 OK");
    }
    else
    {
        strcat(st_line, f_info->err_msg);
    }

    sprintf(res_str,
            "%s\r\nContent-Length: %lu\n%s\nContent-Type: "
            "text/html\r\n\r\n%s\n", st_line, f_info->file_size,
                                              date, code_msg);
    free(date);
    return res_str;
}

/**
 * @brief - Free file_return_t struct elements when needed
 *
 * @param f_info - file_return_t struct to be free'd
 */
void f_ret_destroy(file_return_t * f_info)
{
    if (NULL != f_info->root_dir)
    {
        free(f_info->root_dir);
        f_info->root_dir = NULL;
    }
    if (NULL != f_info->error_dir)
    {
        free(f_info->error_dir);
        f_info->error_dir = NULL;
    }

    if (NULL != f_info->file_path)
    {
        free(f_info->file_path);
        f_info->file_path = NULL;
    }

    if (NULL != f_info->target_file)
    {
        free(f_info->target_file);
        f_info->target_file = NULL;
    }

    if (NULL != f_info->err_msg)
    {
        free(f_info->err_msg);
        f_info->err_msg = NULL;
    }

    if (NULL != f_info)
    {
        free(f_info);
    }
}

/**
 * @brief - Generates response message for 400 and 500 error codes for bonus
 *          feature
 *
 * @param f_info - structure containing file information for reponse msg
 * @return - Generated 400 or 500 html code for response message body
 */
char * read_html_err(file_return_t * f_info)
{
    char html_page[P_MAX];
    char buf[BUFSIZE];
    //Allocate enough memory to fit 2 full buf reads and err_msg
    char * res_str = calloc(3 * BUFSIZE, sizeof(char));
    FILE * fds = NULL;
    
    //Clear memroy before setting name to read
    memset(html_page, 0, P_MAX);
    strncat(html_page, f_info->error_dir, strlen(f_info->error_dir));


    //Set filename to read based on status code set
    if ((b_request == f_info->status) || (forbidden == f_info->status) ||
    (unknown == f_info->status) || (b_method == f_info->status))
    {
        strncat(html_page, "/400.html", 10);
    }
    else if (error == f_info->status)
    {
        strncat(html_page, "/500.html", 10);
    }

    fds = fopen(html_page, "r");

    //Get first of two lines from html error page file
    fgets(buf, BUFSIZE, fds);
    buf[strcspn(buf, "\n")] = 0;
    //Start building response message for 400/500 error
    strncat(res_str, buf, strlen(buf));
    memset(buf, 0, BUFSIZE);
    //Append err_msg to response message
    strncat(res_str, f_info->err_msg, strlen(f_info->err_msg));
    //Get second of two lines from html error page file
    fgets(buf, BUFSIZE, fds);
    buf[strcspn(buf, "\n")] = 0;
    //Append closing of html page to response message
    strncat(res_str, buf, strlen(buf));

    size_t content_length = strlen(res_str);
    f_info->file_size = content_length;

    fclose(fds);

    return res_str;
}
