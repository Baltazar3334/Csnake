#ifndef GAME_H
#define GAME_H

#include "ipc.h"
#include "world.h"

void game_init(SharedGame *game, GameMode mode, int max_time, WorldType wtype, const char *mapfile);
void game_move_snake(SharedGame *game, int id);
void game_spawn_fruits(SharedGame *game);
int game_check_collision(SharedGame *game, int id);
void game_handle_pause(SharedGame *game, int id);

#endif