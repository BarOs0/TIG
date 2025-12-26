cmake_minimum_required(VERSION 3.10)

project(TIG VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(tig_helpers STATIC
    src/recv_file.c
    src/recv_directory.c
    src/send_file.c
    src/send_directory.c
)

add_library(tig_srv_helpers STATIC
    src/daemon.c
)

add_library(tig_cli_helpers STATIC
    src/print_file.c
)

add_executable(TIG_srv
    src/TIG_srv.c
)
target_link_libraries(TIG_srv tig_helpers)
target_link_libraries(TIG_srv tig_srv_helpers)

add_executable(TIG_cli
    src/TIG_cli.c
)
target_link_libraries(TIG_cli tig_helpers)
target_link_libraries(TIG_cli tig_cli_helpers)