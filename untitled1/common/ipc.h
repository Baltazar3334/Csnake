#ifndef IPC_H
#define IPC_H

#include <semaphore.h>

#include "game.h"
#include "world.h"

#define MAX_PLAYERS 4
#define MAP_W 40
#define MAP_H 20

typedef struct {
    int x, y;
} Point;

typedef struct {
    int active;          // hráč je v hre
    int paused;          // hráč pozastavil hru
    int pause_timer;     // sekundy do obnovenia pohybu
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
    Point fruits[MAX_FRUITS];
    Snake snakes[MAX_PLAYERS];

    int running;          // server beží
    int game_time;        // uplynutý čas
    int max_time;         // max čas (len MODE_TIME)
    GameMode mode;        // herný režim
    World world;
    WorldType world_type;
} SharedGame;

int ipc_create();
SharedGame* ipc_attach();
void ipc_destroy();

extern sem_t *game_sem;

#endif