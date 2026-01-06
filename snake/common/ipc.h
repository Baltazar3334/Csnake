#ifndef IPC_H
#define IPC_H

#include <semaphore.h>

#include "../common/config.h"
#include "world.h"



typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

typedef struct {
    int x, y;
} Point;

typedef struct {
    int active;
    int paused;
    int pause_timer;
    int score;
    int length;
    Direction dir;
    Point body[100];
} Snake;

typedef enum {
    WORLD_NO_OBSTACLES,
    WORLD_WITH_OBSTACLES
} WorldType;

typedef enum {
    MODE_STANDARD,
    MODE_TIME
} GameMode;

typedef struct {
    Point fruits[MAX_PLAYERS];
    Snake snakes[MAX_PLAYERS];

    int running;
    int game_time;
    int max_time;
    GameMode mode;
    World world;
    WorldType world_type;
} SharedGame;

int ipc_create();
SharedGame* ipc_attach();
void ipc_destroy();

extern sem_t *game_sem;

#endif