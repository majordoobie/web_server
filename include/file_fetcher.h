#ifndef ASHTI_SRC_FILE_FETCHER_H_
#define ASHTI_SRC_FILE_FETCHER_H_

#include <dirent.h>
#include <ftw.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <http_handler.h>

void file_finder(file_return_t * f_info);
int file_name_checker(const char * file_path, const struct stat * stats, 
                      int flags);
void struct_updater(char * target_path, size_t file_size, mode_t st_mode);

#endif //ASHTI_SRC_FILE_FETCHER_H_
