#ifndef TIG_CLI_H
#define TIG_CLI_H

#include <limits.h>

#define LISTEN_PORT 2025
#define SERVER_NAME "TIG_srv"
#define COMMIT_BUFF_SIZE 64
#define REPOS_BUFF_SIZE 1024
#define NAME_BUFF_SIZE 32

int connection(const char* opt, const char* repo_name, const char* commit);

#endif // TIG_CLI_H