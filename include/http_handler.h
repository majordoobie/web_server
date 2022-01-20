#ifndef ASHTI_SRC_HTTP_HANDLER_H_
#define ASHTI_SRC_HTTP_HANDLER_H_

#include <string.h>
#include <stdbool.h>

typedef enum
{
    BUFSIZE = 1024,
    P_MAX = 4096
} payload_size_t;

typedef enum 
{
	www,
	cgi
} source_type_t;

typedef enum
{
	ok = 200,
	b_request = 400,
	forbidden = 403,
	unknown = 404,
	b_method = 405,
	error = 500
} code_type_t;

typedef struct
{
	char * file_path;
	char * root_dir;
	char * target_file;
	char * err_msg;
    char * error_dir;
	source_type_t source_type;  // www or cgi
	size_t file_size;
	code_type_t status;
    bool read;
    bool execute;
} file_return_t;

void http_parse(file_return_t * f_info, char * src_str);
char * http_datetime(void);
void f_ret_destroy(file_return_t * f_info);
char * http_response(file_return_t * f_info, char * code_msg);
char * read_html_err(file_return_t * f_info);


#endif //ASHTI_SRC_HTTP_HANDLER_H_
