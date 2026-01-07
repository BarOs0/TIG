# TIG

## Project Description
TIG is a tool for transferring directories and files between a client and a server using IPv6 in local network.

## Requirements
- Linux system (Ubuntu)
- C compiler (e.g. gcc)
- CMake >= 3.10
- POSIX libraries (standard on Linux)

## Directory Structure
- `src/` — program source code (C)
- `include/` — header files
- `srv/` — server data (backups, commits, list, repos)
```sh
!!! The administrator must supervise the data from srv/data, e.g. deleting dead repositories, and records related to this repositories. !!!
```
- `srv/data/backups` — repository version from last push
- `srv/data/commits` — a directory of time-stamped commits
- `srv/data/repos` — directory with the latest repositories
- `srv/data/list` — list of repositories - this file appears after first push

## Build Instructions
```sh
mkdir build
cd build
cmake ..
make
```
The executables `TIG_cli` (client) and `TIG_srv` (server) will appear in the `build/` directory.

### Client arguments
```sh
./TIG_cli <repos/discover> or <commit> <repo name> <message> or <pull/push> <repo name> or <read> <repo name>
```

### Server administrator manual
- Check server interface configurations and verify IPv6 addresess (for UDP dicovery and TCP conection)
```sh
ifconfig
```
- If it is necessery add global-link IPv6 address for TCP communication
```sh
ifconfig <if name> inet6 addif <IPv6 address>/<prefix> up
```
- Check server interface name for UDP mcast discovery (#define MCAST_IF in mcast_respond.h)
- Check server IPv6 mulitcast address (ff02:: ... for local link) (#define MCAST_ADDR in mcast_respond.h)

- Disable fierwall for tcp on 2025 port:
```sh
sudo ip6tables -I INPUT -p tcp --dport 2025 -j ACCEPT
```

- Disable firewall for udp on 2026 port:
```sh
sudo ip6tables -I INPUT -p udp --dport 2026 -j ACCEPT
```

- Configure your server data storage path (#define REPOS_PATH in TIG_srv.h):
```sh
/home/<user>/<TIG>/srv/data
```

- Run TIG server daemon:
```sh
sudo ./TIG_srv
```

- Check if TIG daemon works:
```sh
sudo ps aux | grep TIG_srv
```

- Check syslog logs:
```sh
sudo grep TIG_srv /var/log/syslog 
```
- Reset syslog history for TIG server:
```sh
sudo truncate -s 0 /var/log/syslog
```

- Terminate TIG server process:
```sh
sudo ps aux | grep TIG_srv
sudo kill -9 <daemon PID>
```

### Client manual
- Check your interface configurations and verify IPv6 adressess (for UDP dicovery and TCP conection)
```sh
ifconfig
```
- If it is necessery add local-link IPv6 address for UDP mcast discovery
- If it is necessery add global-link IPv6 address for TCP communication
```sh
ifconfig <if name> inet6 addif <IPv6 address>/<prefix> up
```
- Check client interface name for UDP mcast discovery (#define MCAST_IF in mcast_discover.h)
- Check client IPv6 multicast address (ff02:: ... for local link) (#define MCAST_ADDR in mcast_discover.h)

- [optional] Add an alias for TIG_cli application:
```sh
vim ~/.bashrc

<INSERT>
<alias TIG='/installation_path/TIG_cli'>
<ESC>
:wq
```

- Run TIG_cli with discover command:
```sh
./TIG_cli discover
```
- If you already have alias:
```sh
TIG discover
```

- If client discovered TIG server - configure /etc/hosts file for DNS:
```sh
vim /etc/hosts

<INSERT>
<[Server address from discovery command (global-link)]    TIG_srv>
<ESC>
:wq
```
- Now you can use TIG system

# Documentation
For API detatils create a doxygen documentation from the project root:
```sh
doxygen -g
doxygen Doxyfile
```

