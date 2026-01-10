#ifndef IPC_H
#define IPC_H

#include <semaphore.h>
#include "../common/config.h"
#include "world.h"

// Typy
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
    int invincible;
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

#define MAGIC_ID 0xDEADBEEF  // magické číslo na identifikáciu servera

typedef struct {
    int magic;             // pridáme sem na overenie servera
    Point fruits[MAX_FRUITS];
    Snake snakes[MAX_PLAYERS];
    int running;
    int game_time;
    int shutdown;
    int max_time;
    GameMode mode;
    World world;
    WorldType world_type;
} SharedGame;

// IPC funkcie
int ipc_create(const char* server_name);
SharedGame* ipc_attach(const char* server_name);
void ipc_destroy(const char* server_name);
void ipc_detach(SharedGame *ptr);

extern sem_t *game_sem;

#endif