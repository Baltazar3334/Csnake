#include "../common/config.h"
#include "render_text.h"
#include "../common/ipc.h"
#include "../common/game.h"
#include <stdio.h>
#include <unistd.h>


void render_init_text() {
    // Textový render nepotrebuje nič špeciálne
}

void render_cleanup_text() {
    // Textový render nepotrebuje nič špeciálne
}

// Vymazanie obrazovky (ANSI escape sekvencia)
static void clear_screen() {
    printf("\033[H\033[J");
}

void render_game_text(SharedGame *game, int id) {
    clear_screen();

    // Vykreslenie mapy
    for (int y = 0; y < game->world.height; y++) {
        for (int x = 0; x < game->world.width; x++) {
            if (game->world.grid[y][x] == WORLD_WALL)
                printf("#");
            else
                printf(".");
        }
        printf("\n");
    }

    // Vykreslenie ovocia
    for (int i = 0; i < MAX_FRUITS; i++) {
        printf("\033[%d;%dH", game->fruits[i].y + 1, game->fruits[i].x + 1);
        printf("O");
    }

    // Vykreslenie hadíkov
    for (int i = 0; i < MAX_PLAYERS; i++) {
        Snake *s = &game->snakes[i];
        if (!s->active) continue;

        // hlava
        printf("\033[%d;%dH", s->body[0].y + 1, s->body[0].x + 1);
        printf("H");

        // telo
        for (int j = 1; j < s->length; j++) {
            printf("\033[%d;%dH", s->body[j].y + 1, s->body[j].x + 1);
            printf("o");
        }
    }

    // jednoduchý HUD
    Snake *me = &game->snakes[id];
    printf("\033[%d;0H", game->world.height + 2);
    if (me->paused)
        printf("[PAUZA] Navrat do hry pomocou [p] Score: %d\n", me->score);
    else if (me->pause_timer > 0)
        printf("Navrat do hry za %d s\n", me->pause_timer);
    else if (game->mode == MODE_TIME)
        printf("Ovladanie: [WASD] | Ukoncenie: [Q] | Pauza: [P] | Score: %d | Cas do konca: %d s\n", me->score, game->max_time - game->game_time);
    else
        printf("Ovladanie: [WASD] | Ukoncenie: [Q] | Pauza: [P] | Score: %d | Cas hry: %d s\n", me->score, game->game_time);

    fflush(stdout);
}