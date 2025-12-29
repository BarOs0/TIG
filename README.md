# TIG

## Project Description
TIG is a tool for transferring directories and files between a client and a server using IPv6 networking. It enables backups, directory synchronization, and automated file management.

## Requirements
- Linux system
- C compiler (e.g. gcc)
- CMake >= 3.10
- POSIX libraries (standard on Linux)

## Directory Structure
- `src/` — program source code (C)
- `include/` — header files
- `srv/` — server data (backups, commits, repos)

## Build Instructions
```sh
mkdir -p build
cd build
cmake ..
make
```
The executables `TIG_cli` (client) and `TIG_srv` (server) will appear in the `build/` directory.

## Running the Program
### Server
```sh
./build/TIG_srv
```

### Client
```sh
./build/TIG_cli <repos/discover> or <commit> <repo name> <message> or <pull/push> <repo name>
```

### Useful commands for administrator
```sh
sudo ip6tables -I INPUT -p tcp --dport 2025 -j ACCEPT //Disable firewall on 2025 port
sudo grep TIG_srv /var/log/syslog //Check syslog logs for TIG_srv
sudo truncate -s 0 /var/log/syslog //Reset syslog logs for TIG_srv
sudo ps aux | grep TIG_srv //Check TIG_srv daemon PID
```

### Hints for user
```sh
Add IPv6 server address in /etc/hosts, use domain: "TIG_srv" or change client configuration [#define SERVER_NAME] in include/TIG_cli.h
```

