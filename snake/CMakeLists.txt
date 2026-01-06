cmake_minimum_required(VERSION 4.1)
project(SnakeGame C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -g -pthread")

# Common súbory
set(SRC_COMMON
        common/ipc.c
        common/game.c
        common/world.c
)

set(HDR_COMMON
        common/ipc.h
        common/game.h
        common/world.h
)

# Klient
set(SRC_CLIENT
        client/client.c
        client/input.c
        client/render_text.c
        ${SRC_COMMON}
)

set(HDR_CLIENT
        client/input.h
        client/render_text.h
        ${HDR_COMMON}
)

add_executable(client ${SRC_CLIENT} ${HDR_CLIENT})
target_link_libraries(client pthread)

# Server
set(SRC_SERVER
        server/server.c
        ${SRC_COMMON}
)

set(HDR_SERVER
        ${HDR_COMMON}
)

add_executable(server ${SRC_SERVER} ${HDR_SERVER})
target_link_libraries(server pthread)