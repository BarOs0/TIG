#include "TIG_cli.h"
#include "recv_directory.h"
#include "send_directory.h"
#include "print_file.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int connection(const char* opt, const char* repo_name, const char* commit) {
    int n = 0, sockfd = 0;
    struct addrinfo hints, *res, *rp;
    int err;
    const char *server_name = SERVER_NAME;
    char cmd;
    char commit_buff[COMMIT_BUFF_SIZE] = {0};
    char repos_buff[REPOS_BUFF_SIZE] = {0};
    char name_buff[NAME_BUFF_SIZE] = {0};

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;

    char portstr[16];
    snprintf(portstr, sizeof(portstr), "%d", LISTEN_PORT);

    if ((err = getaddrinfo(server_name, portstr, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo() error for: %s\n", gai_strerror(err));
        return -1;
    }

    for (rp = res; rp != NULL; rp = rp->ai_next){
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd == -1)
            continue;

        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;

        close(sockfd);
    }

    if (rp == NULL){
        fprintf(stderr, "Could not connect to %s\n", server_name);
        freeaddrinfo(res);
        return -1;
    }

    char cwd[PATH_MAX];
    if(getcwd(cwd, PATH_MAX) == NULL){
        fprintf(stderr, "getcwd() error: %s\n", strerror(errno));
        return -1;
    }

    if(repo_name == NULL){
        if(strcmp(opt, "repos") == 0){
            cmd = 'R';
            if(write(sockfd, &cmd, 1) < 0){
                fprintf(stderr, "TIG_cli.c repos write() error: %s\n", strerror(errno));
                return -1;
            }
            print_file(sockfd);
        }
    }
    else{
        if(strcmp(opt, "commit") == 0){
            cmd = 'C';
            if(write(sockfd, &cmd, 1) < 0){
                fprintf(stderr, "TIG_cli.c commit write() error: %s\n", strerror(errno));
                return -1;
            }
            strcpy(name_buff, repo_name);
            if(write(sockfd, name_buff, NAME_BUFF_SIZE) < 0){
                fprintf(stderr, "TIG_cli.c commit write() error: %s\n", strerror(errno));
                return -1;
            }
            if(strlen(commit) >= COMMIT_BUFF_SIZE){
                fprintf(stderr, "Commit message is too long\n");
                return -1;
            }
            strcpy(commit_buff, commit);
            if(write(sockfd, commit_buff, COMMIT_BUFF_SIZE) < 0){
                fprintf(stderr, "TIG_cli.c commit message write() error: %s\n", strerror(errno));
                return -1;
            }
        }
        else if(strcmp(opt, "pull") == 0){
            cmd = 'U';
            if(write(sockfd, &cmd, 1) < 0){
                fprintf(stderr, "TIG_cli.c pull write() error: %s\n", strerror(errno));
                return -1;
            }
            strcpy(name_buff, repo_name);
            if(write(sockfd, name_buff, NAME_BUFF_SIZE) < 0){
                fprintf(stderr, "TIG_cli.c name in pull write() error: %s\n", strerror(errno));
                return -1;
            }
            recv_directory(sockfd, cwd);
        }
        else if(strcmp(opt, "push") == 0){
            cmd = 'P';
            if(write(sockfd, &cmd, 1) < 0){
                fprintf(stderr, "TIG_cli.c push write() error: %s\n", strerror(errno));
                return -1;
            }
            if(strlen(repo_name) >= NAME_BUFF_SIZE){
                fprintf(stderr, "Repo name is too long\n");
                return -1;
            }
            strcpy(name_buff, repo_name);
            if(write(sockfd, name_buff, NAME_BUFF_SIZE) < 0){
                fprintf(stderr, "TIG_cli.c name in push write() error: %s\n", strerror(errno));
                return -1;
            }
            send_directory(sockfd, cwd);
        }
    }

    fprintf(stderr, "OK\n");
    freeaddrinfo(res);
    close(sockfd);

    return 0;
}

int main(int argc, char **argv){

    

    if((argc == 2) && (strcmp(argv[1], "repos") == 0)){
        return connection(argv[1], NULL, NULL);
    }
    else if((argc == 2) && (strcmp(argv[1], "discover") == 0)){
        return mcast_discover();
    }
    else if(argc == 3 && (strcmp(argv[1], "pull") == 0 || strcmp(argv[1], "push") == 0)){
        return connection(argv[1], argv[2], NULL);
    }
    else if(argc == 4 && (strcmp(argv[1], "commit") == 0)){
        return connection(argv[1], argv[2], argv[3]);
    }
    else{
        fprintf(stderr, "ERROR: usage: ./TIG_cli <repos/discover> or ./TIG_cli <commit> <repo name> <message> or ./TIG_cli <pull/push> <repo name>\n");
    }
    
    return 0;
}