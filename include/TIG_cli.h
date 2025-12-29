#ifndef TIG_CLI_H
#define TIG_CLI_H

#include <limits.h>

#define LISTEN_PORT 2025 // Server listening port
#define SERVER_NAME "TIG_srv" // Server domain (user configurable)
#define COMMIT_BUFF_SIZE 64 // Commit message buffer
#define NAME_BUFF_SIZE 32 // File or directory name buffer

int connection(const char* opt, const char* repo_name, const char* commit); // Connection with server function

#endif // TIG_CLI_H