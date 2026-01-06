#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../common/ipc.h"
#include "../common/game.h"

int main(int argc, char **argv) {
    // Vytvorenie shared memory
    if (ipc_create() == -1) {
        exit(1);
    }

    SharedGame *game = ipc_attach();
    if (!game) {
        fprintf(stderr, "Nepodarilo sa pripojit k shared memory\n");
        exit(1);
    }

    // Inicializácia hry
    game_init(game, MODE_TIME, 60, WORLD_NO_OBSTACLES, NULL);

    game->running = 1;

    printf("Server spusteny\n");

    // Hlavny loop servera (jednoduchý)
    while (game->running) {
        // Tu by sa pohybovali hadíky, kontrola kolízií, atď.
        sleep(1);
        game->game_time++;
        if (game->mode == MODE_TIME && game->game_time >= game->max_time)
            game->running = 0;
    }

    printf("Server ukonceny\n");
    ipc_destroy();
    return 0;
}