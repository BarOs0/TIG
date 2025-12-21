cmake_minimum_required(VERSION 3.10)

project(TIG VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(TIG_srv
    src/TIG_srv.c
    src/daemon.c
)

add_executable(TIG_cli
    src/TIG_cli.c
)