#include "../common/config.h"
#include "game.h"
#include <stdio.h>
#include "world.h"
#include <stdlib.h>
#include <time.h>

static int occupied(SharedGame *game, int x, int y) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!game->snakes[i].active) continue;
        for (int j = 0; j < game->snakes[i].length; j++) {
            if (game->snakes[i].body[j].x == x &&
                game->snakes[i].body[j].y == y)
                return 1;
        }
    }
    return 0;
}

void game_spawn_fruits(SharedGame *game) {
    srand(time(NULL));
    for (int i = 0; i < MAX_FRUITS; i++) {
        do {
            game->fruits[i].x = rand() % MAP_W;
            game->fruits[i].y = rand() % MAP_H;
        } while (occupied(game,
                 game->fruits[i].x,
                 game->fruits[i].y));
    }
}

void game_init(SharedGame *game,
               GameMode mode,
               int max_time,
               WorldType wtype,
               const char *mapfile) {

    game->mode = mode;
    game->max_time = max_time;
    game->game_time = 0;
    game->running = 1;
    game->world_type = wtype;

    /* ===== SVET ===== */
    if (wtype == WORLD_WITH_OBSTACLES) {
        if (!world_load(&game->world, mapfile)) {
            printf("Chyba pri nacitani mapy\n");
            exit(1);
        }
    } else {
        game->world.width = MAP_W;
        game->world.height = MAP_H;
    }

    /* ===== HRÁČI ===== */
    for (int i = 0; i < MAX_PLAYERS; i++) {
        game->snakes[i].active = 0;
        game->snakes[i].paused = 0;
        game->snakes[i].pause_timer = 0;
    }

    /* ===== OVOCIE ===== */
    game_spawn_fruits(game);
}

void game_move_snake(SharedGame *game, int id) {
    Snake *s = &game->snakes[id];

    /* posun tela */
    for (int i = s->length - 1; i > 0; i--) {
        s->body[i] = s->body[i - 1];
    }

    /* pohyb hlavy */
    switch (s->dir) {
        case DIR_UP:    s->body[0].y--; break;
        case DIR_DOWN:  s->body[0].y++; break;
        case DIR_LEFT:  s->body[0].x--; break;
        case DIR_RIGHT: s->body[0].x++; break;
    }

    /* ===== SVET BEZ PREKÁŽOK – WRAP ===== */
    if (game->world_type == WORLD_NO_OBSTACLES) {
        if (s->body[0].x < 0)
            s->body[0].x = game->world.width - 1;
        if (s->body[0].x >= game->world.width)
            s->body[0].x = 0;

        if (s->body[0].y < 0)
            s->body[0].y = game->world.height - 1;
        if (s->body[0].y >= game->world.height)
            s->body[0].y = 0;
    }

    /* ===== OVOCIE ===== */
    for (int i = 0; i < MAX_FRUITS; i++) {
        if (s->body[0].x == game->fruits[i].x &&
            s->body[0].y == game->fruits[i].y) {

            s->length++;
            s->score++;
            game_spawn_fruits(game);
            }
    }
}

int game_check_collision(SharedGame *game, int id) {
    Snake *s = &game->snakes[id];

    int x = s->body[0].x;
    int y = s->body[0].y;

    /* ===== PREKÁŽKY ===== */
    if (game->world_type == WORLD_WITH_OBSTACLES) {
        if (world_is_wall(&game->world, x, y)) {
            return 1;
        }
    }

    /* ===== SÁM DO SEBA ===== */
    for (int i = 1; i < s->length; i++) {
        if (x == s->body[i].x &&
            y == s->body[i].y) {
            return 1;
            }
    }

    /* ===== INÝ HADÍK ===== */
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (i == id || !game->snakes[i].active)
            continue;

        Snake *o = &game->snakes[i];
        for (int j = 0; j < o->length; j++) {
            if (x == o->body[j].x &&
                y == o->body[j].y) {
                return 1;
                }
        }
    }

    return 0;   // bez kolízie
}

void game_handle_pause(SharedGame *game, int id) {
    Snake *s = &game->snakes[id];

    if (s->paused && s->pause_timer > 0) {
        s->pause_timer--;
    }

    if (!s->paused && s->pause_timer > 0) {
        s->pause_timer--;
    }
}