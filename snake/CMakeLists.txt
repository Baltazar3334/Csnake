cmake_minimum_required(VERSION 4.1)
project(Csnake C)

set(CMAKE_C_STANDARD 11)

add_executable(server
        server/server.c
        common/ipc.c
        common/game.c
        common/world.c
)

add_executable(client
        client/client.c
        common/ipc.c
        common/game.c
        common/world.c
        common/config.h
        common/config.h
        client/render_text.c
        client/input.c
        client/input.h
)