#include <c_subprocess.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <log4j.h>

enum {BUFF_SZ = 512};

static char * read_pipe(int pipe_fd);

/*!
 * Take in an array of string arrays and attempt to execute what they have.
 * Report back the stdout to the caller and report stderror to the logs
 *
 * @param arguments Array of strings to execute
 * @return NULL if a stdout is received or a pointer to a string containing
 * the stdout
 */
char * c_subprocess(file_return_t * f_info, char ** arguments)
{

    //500 code string
    char i_err[] = "500 Internal Server Error";

    // create pipes for communications
    int out_pipe[2];
    int err_pipe[2];

    int result = 0;

    result = pipe(out_pipe);
    if (-1 == result)
    {
        perror("PIPE");
        return NULL;
    }

    result = pipe(err_pipe);
    if (-1 == result)
    {
        perror("PIPE");
        return NULL;
    }


    int pid = fork();
    if (0 > pid)
    {
        perror("FORK");
        return NULL;
    }

    // child process
    if (0 == pid)
    {
        // close the reading ends, we are only writing
        close(out_pipe[0]);
        close(err_pipe[0]);

        // replace the stdout and error to use the pipes
        dup2(out_pipe[1], STDOUT_FILENO);
        dup2(err_pipe[1], STDERR_FILENO);


        // exec with v: vector args p: resolve path if possible
        if (NULL == arguments)
        {
            execlp(
                f_info->file_path,
                f_info->file_path,
                NULL
            );
        }
        else
        {
            execvp(
                arguments[0],
                arguments
            );
        }
        perror("FORK\n\n");
        // we get here when exec was not possible and errno is set
        exit(errno);
    }

    // parent process
    else
    {
        // close the writing ends, we only want to read
        close(out_pipe[1]);
        close(err_pipe[1]);
        char log_msg[512];

        // Wait for child to finish successfully
        int wstatus = 0;
        while (-1 == waitpid(pid, &wstatus, 0))
        {
            if (EINTR == errno)
            {
                log_error("[c_subproces] parent interrupted - restarting");
                continue;
            }
            else
            {
                sprintf(log_msg, "[c_subprocess] while waiting, received "
                                 "status code: %d : %s", errno, strerror
                                 (errno));
                log_error(log_msg);
                break;
            }
        }

        if (WIFEXITED(wstatus))
        {
            // if we get here then we know the program executed normally
            int returned = WEXITSTATUS(wstatus);
            if (0 != returned)
            {
                sprintf(
                    log_msg,
                    "[c_subprocess] child process reported \"%s\" while "
                    "attempting to execute \"%s\"",
                                 strerror(returned),
                                 arguments[0]);
                log_error(log_msg);
            }
        }
        else if (WIFSIGNALED(wstatus) || (WIFSTOPPED(wstatus)))
        {
            log_error("[c_process] child process exited due to signal");
        }
        else
        {
            log_error("[c_process] unknown error occurred processing child "
                      "process");
        }

        char * std_out = read_pipe(out_pipe[0]);
        char * std_err = read_pipe(err_pipe[0]);

        if (0 < strlen(std_err))
        {
            //Set 500 code and err_msg
            //500 Internal Server Error
            f_info->status = error;
            f_info->err_msg = calloc(1, sizeof(char) * strlen(i_err) + 1);
            strncpy(f_info->err_msg, i_err, strlen(i_err));
            log_error(std_err);
        }
        free(std_err);

        return std_out;
    }
}

/*!
 * Read data from a file descriptor if there is any. If there is data, read it
 * and return a allocated string
 *
 * @param pipe_fd any file descriptor for reading
 * @return NULL if there is no data or a pointer to a string
 */
static char * read_pipe(int pipe_fd)
{
    ssize_t bytes_read = 0;
    char * output = calloc(1, sizeof(char));

    size_t output_len = 0;

    char buffer[BUFF_SZ];
    memset(buffer, '\0', BUFF_SZ);

    bytes_read = read(pipe_fd, buffer, BUFF_SZ);
    while (0 < bytes_read)
    {
        // add null terminator to the read string
        buffer[bytes_read] = '\0';

        // calculate the new length of the string and realloc output
        output_len += (strlen(buffer) + 1);
        output = realloc(output, sizeof(char) * output_len);

        // copy the new contents to the output
        strncat(output, buffer, bytes_read);

        // loop
        bytes_read = read(pipe_fd, buffer, BUFF_SZ);
    }

    close(pipe_fd);
    return output;
}

