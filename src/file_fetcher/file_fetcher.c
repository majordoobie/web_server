#define _GNU_SOURCE

#include <file_fetcher.h>

file_return_t * stat_struct;

/**
 * @brief - Update the file_return_t struct.
 *
 * @param target_path - full path of the target file
 * @param file_size - size of the target file
 * @param st_mode - permissions of the file
 * 
 * @return None
 */
void struct_updater(char * target_path, size_t file_size, mode_t st_mode)
{
    //Set the file_path var in file_return_t struct
    size_t size = strlen(target_path);
    stat_struct->file_path = calloc(size + 1, sizeof(char));
    strncpy(stat_struct->file_path, target_path, size);

    //Set file size var in file_return_t struct
    stat_struct->file_size = file_size;

    //Set the perms for read/execute as true if set in file_return_t struct
    if (S_IRUSR & st_mode)
    {
        stat_struct->read = true;
    }
    if (S_IXUSR & st_mode)
    {
        stat_struct->execute = true;
    }
    
}

/**
 * @brief checks each file name and finds the match to 
 *        the file_path we are searching for
 * 
 * @param file_path - current file path in the ftw traversal.
 * @param stats - stat struct on the current file in ftw traversal.
 * @param flags - flags from ftw with details on the current file.
 * @return int - returns 1 if file was found, 0 if the file not found 
 */

int file_name_checker(const char * file_path, const struct stat * stats,
                      int flags)
{
    //Status code return for ftw
    int ret_val = 0;

    //Allocate str to hold abs path to server cwd
    char * cwd_root = calloc(strlen(stat_struct->root_dir) + 2, sizeof(char));
    strncat(cwd_root, stat_struct->root_dir, strlen(stat_struct->root_dir));
    strncat(cwd_root, "/", 2);

    //Allocate str to hold abs path to target file with GET request
    char * message_cwd = calloc(strlen(cwd_root) +
                       strlen(stat_struct->target_file) + 2,
                       sizeof(char));
    strncat(message_cwd, cwd_root, strlen(cwd_root));
    strncat(message_cwd, stat_struct->target_file, 
            strlen(stat_struct->target_file));

    //Allocate str to store resolved abs path to file
    char * abs_path = calloc(strlen(message_cwd) + 1, sizeof(char));
    
    //Put resolved abs path into abs_path and err check with rp_res
    realpath(message_cwd, abs_path);

    //Get resolved abs_path of file_path from ftw
    char actual_file_path[P_MAX + 1];
    realpath(file_path, actual_file_path);

    if(FTW_F == flags)
    {
        //Check if cgi or www
        if (NULL != strstr(abs_path, "/cgi-bin")) 
        {
            stat_struct->source_type = cgi;
            if (0 == strcmp(abs_path, actual_file_path))
            {
                struct_updater(abs_path, stats->st_size, stats->st_mode);
                ret_val = 1;
            }
        } 
        else
        {   
            stat_struct->source_type = www;
            if (0 == strcmp(abs_path, actual_file_path))
            {
                struct_updater(abs_path, stats->st_size, stats->st_mode);
                ret_val = 1;
            }
        }
    }
    free(cwd_root);
    cwd_root = NULL;
    free(message_cwd);
    message_cwd = NULL;
    free(abs_path);
    abs_path = NULL;
    return ret_val;
}

/**
 * @brief sets up the ftw function and sets status codes
 * 
 * @param root_dir - character array of the root directory of the search
 * @param f_info - struct with the file info.
 * 
 * @return None
 */
void file_finder(file_return_t * f_info)
{
    //character strings for err_msg
    char f_err[] = "403 Forbidden";
    char u_err[] = "404 Not Found";

    //Make file_return_t global to file for ftw
    stat_struct = f_info;

    //Status code var for ftw entry/exit
    int ftw_status;

    //Check if request has cgi-bin in it to set root dir as
    //dirname -> /cgi-bin/ or dirname/www -> everything else
    size_t root_dir_len = strlen(stat_struct->root_dir) + 1;
    if ( NULL == strstr(stat_struct->target_file, "/cgi-bin/"))
    {    
        stat_struct->root_dir = realloc(stat_struct->root_dir, root_dir_len + 5);
        strncat(stat_struct->root_dir, "/www", 5);
    }

    ftw_status = ftw(stat_struct->root_dir,file_name_checker, 20);
    //Verify if the file path, source type, and read perms are set
    //correctly for www file. If yes 200 OK page.
    if (NULL != stat_struct->file_path && www == stat_struct->source_type
        && true == stat_struct->read)
    {
        stat_struct->status = ok;

    }
    //Else Verify CGI requirements
    else if (NULL != stat_struct->file_path && cgi == stat_struct->source_type)
    {
        //If the file in CGI can be executed, 200 OK page
        if (true == stat_struct->execute)
        {
            stat_struct->status = ok;
        }
        //Else 403 Forbidden Page
        else
        {
            stat_struct->status = forbidden;
            stat_struct->err_msg = calloc(strlen(f_err) + 1, sizeof(char));
            strncpy(stat_struct->err_msg, f_err, strlen(f_err));
        }

    }
    //If ftw returned 0 or 2, file not found or not in server path
    else if (0 == ftw_status || 2 == ftw_status)
    {
        stat_struct->status = unknown;
        stat_struct->err_msg = calloc(strlen(u_err) + 1, sizeof(char));
        strncpy(stat_struct->err_msg, u_err, strlen(u_err));
    }
}
