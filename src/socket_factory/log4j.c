#include <log4j.h>
#include <sys/syslog.h>

void log_info(const char *msg)
{
        openlog("ashti-server", LOG_PID|LOG_PERROR, LOG_USER);
        syslog(LOG_INFO, " [INFO]: %s", msg);
        closelog();
}

void log_error(const char *msg)
{
    openlog("ashti-server", LOG_PID|LOG_PERROR, LOG_USER);
    syslog(LOG_ERR, "[ERROR]: %s", msg);
    closelog();
}
