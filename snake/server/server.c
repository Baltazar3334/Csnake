#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "../common/config.h"
#include "../common/ipc.h"
#include "../common/game.h"

SharedGame *game = NULL;

void handle_sigint(int sig) {
    (void)sig;
    if (game) {
        ipc_destroy();
        printf("\nServer ukončený.\n");
    }
    exit(0);
}

int main(void) {
    signal(SIGINT, handle_sigint);

    ipc_create();
    game = ipc_attach();
    if (!game) {
        perror("Pripajanie k zdieľanej pamäti zlyhalo");
        exit(1);
    }

    game_init(game, MODE_TIME, 60, WORLD_NO_OBSTACLES, NULL);

    printf("Server spustený.\n");

    while (game->running) {
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (!game->snakes[i].active || game->snakes[i].paused)
                continue;

            game_move_snake(game, i);

            if (game_check_collision(game, i)) {
                printf("Hráč %d prehral.\n", i);
                game->snakes[i].active = 0;
            }
        }

        usleep(200000);
        game->game_time++;

        if (game->mode == MODE_TIME && game->game_time >= game->max_time)
            game->running = 0;
    }

    printf("Hra skončila.\n");
    ipc_destroy();
    return 0;
}