#ifndef TIG_CLI_H
#define TIG_CLI_H

#include <limits.h>

#define LISTEN_PORT 2025 ///< Server listening port
#define SERVER_NAME "TIG_srv" ///< Server domain (user configurable)
#define COMMIT_BUFF_SIZE 512 ///< Commit message buffer
#define NAME_BUFF_SIZE 32 ///< File or directory name buffer

/**
 * @brief Connection with server function
 * @param opt Operation type (repos, commit, pull, push, read)
 * @param repo_name Repository name
 * @param commit Commit message
 * @return 0 on success, -1 on error
 */
int connection(const char* opt, const char* repo_name, const char* commit);

#endif // TIG_CLI_H