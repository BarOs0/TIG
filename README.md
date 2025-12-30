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
```sh
The administrator must supervise the data from srv/data, e.g. deleting dead repositories, and records related to this repositories.
```
- `srv/data/backups` — repository version from last push
- `srv/data/commits` — a directory of time-stamped commits
- `srv/data/repos` — directory with the latest repositories
- `srv/data/list` — list of repositories / this file appears after first push 

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

### Client arguments
```sh
./build/TIG_cli <repos/discover> or <commit> <repo name> <message> or <pull/push> <repo name>
```

### Server administrator manual
```sh
1. Check your interface configurations and verify
    ifconfig
1.1 If it is necessery add global-link IPv6 address
    ifconfig <if name> inet6 addif <IPv6 address>/<prefix> up
1.2 Check server interface name (#define MCAST_IF in mcast_respond.h)
1.3 Check server IPv6 address (#define MCAST_ADDR in mcast_respond.h)
2. Disable fierwall for tcp on 2025 port:
    sudo ip6tables -I INPUT -p tcp --dport 2025 -j ACCEPT
3. Disable firewall for udp on 2026 port:
    sudo ip6tables -I INPUT -p udp --dport 2026 -j ACCEPT
4. Configure your server data storage path (#define REPOS_PATH in TIG_srv.h):
    /home/<user>/<TIG>/srv/data
5. Run TIG server daemon:
    sudo ./TIG_daemon
5.1 Check if TIG daemon works:
    sudo ps aux | grep TIG_srv
6. Check syslog logs:
    sudo grep TIG_srv /var/log/syslog 
6.1 Reset syslog history for TIG server:
    sudo truncate -s 0 /var/log/syslog
7. Disable TIG server:
    sudo ps aux | grep TIG_srv
    sudo kill -9 <daemon PID>
```

### Client manual
```sh
1. Check your interface configurations and verify
    ifconfig
1.1 If it is necessery add global-link IPv6 address
    ifconfig <if name> inet6 addif <IPv6 address>/<prefix> up
1.2 Check client interface name (#define MCAST_IF in mcast_discover.h)
1.3 Check client IPv6 address (#define MCAST_ADDR in mcast_discover.h)
2. Run TIG_cli with discover command:
    ./TIG_cli discover
3. If client discovered TIG server - configure /etc/hosts file for DNS:
    vim /etc/hosts
    <INSERT>
    <Server address from discovery command>    TIG_srv/<domain>
    <INSERT>
    :wq
4. Now you can use TIG system
```

