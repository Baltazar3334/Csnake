#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#include "../common/ipc.h"
#include "../common/game.h"

int main(void) {
    SharedGame *game = ipc_attach();
    if (!game) {
        perror("Nepodarilo sa pripojit k zdieľanej pamäti");
        exit(1);
    }

    // Inicializácia hry
    game_init(game, MODE_STANDARD, 0, WORLD_NO_OBSTACLES, NULL);
    game->running = 1;
    game->shutdown = 0;  // flag pre klientov

    printf("Server spusteny. Hra bezi.\n");

    while (1) {
        sem_wait(game_sem);

        // Pohyb hadikov a kontrola kolízii
        for (int i = 0; i < MAX_PLAYERS; i++) {
            Snake *s = &game->snakes[i];
            if (!s->active || s->paused)
                continue;

            game_move_snake(game, i);

            if (game_check_collision(game, i)) {
                printf("Hadik %d zomrel.\n", i);
                s->active = 0;
            }
        }

        // Generovanie ovocia
        game_spawn_fruits(game);

        // Inkrementacia casu
        game->game_time++;

        // Spočítanie aktívnych hráčov
        int active_players = 0;
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (game->snakes[i].active)
                active_players++;
        }

        // Ak už nie sú aktívni hráči, ukonči hru
        if (active_players == 0) {
            printf("Vsetci hraci zomreli. Ukoncenie hry.\n");
            game->running = 0;
            game->shutdown = 1;  // signal pre klientov
            sem_post(game_sem);
            break;
        }

        sem_post(game_sem);
        usleep(200000); // 200ms medzi krokmi
    }

    // Počkame chvíľu, aby sa klienti stihli odmapovať
    sleep(1);

    // Cleanup shared memory a semaforu
    ipc_destroy();

    printf("Server ukoncil hru a uvolnil zdroje.\n");
    return 0;
}