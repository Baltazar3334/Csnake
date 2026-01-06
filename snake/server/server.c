#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#include "../common/ipc.h"
#include "../common/game.h"

int main(void) {

    int fd = ipc_create();
    SharedGame *game = ipc_attach();

    if (!game) {
        perror("Nepodarilo sa pripojit k zdieľanej pamäti");
        exit(1);
    }

    // Inicializácia hry
    game_init(game, MODE_STANDARD, 0, WORLD_NO_OBSTACLES, NULL);
    game->running = 1;

    printf("Server spusteny. Hra bezi.\n");

    // Hlavný herný loop
    while (1) {
        sem_wait(game_sem);

        int active_players = 0;
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (game->snakes[i].active)
                active_players++;
        }

        // Ak uz nie su aktivni hraci, ukonci hru
        if (active_players == 0) {
            game->running = 0;
            sem_post(game_sem);
            break;
        }

        // Pohyb hadikov a kontrola kolizii
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (!game->snakes[i].active || game->snakes[i].paused)
                continue;

            game_move_snake(game, i);

            if (game_check_collision(game, i)) {
                printf("Hadik %d zomrel.\n", i);
                game->snakes[i].active = 0;
            }
        }

        // Pripadne generovanie ovocia
        game_spawn_fruits(game);

        // Inkrementacia casu
        game->game_time++;

        sem_post(game_sem);

        usleep(200000); // 200ms medzi krokmi hry
    }

    printf("Hra skončila. Server ukončený.\n");

    ipc_destroy(); // uvolni zdieľanú pamäť a semafor
    return 0;
}