#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#include "../common/ipc.h"
#include "../common/game.h"

#define FRUIT_SPAWN_INTERVAL 50 // každých 50 ticov = ~10 sekúnd

int main(void) {
    // ===== Vytvorenie shared memory a semaforu =====
    if (ipc_create() == -1) {
        fprintf(stderr, "Nepodarilo sa vytvorit shared memory a semafor\n");
        exit(1);
    }

    // ===== Pripojenie servera k shared memory =====
    SharedGame *game = ipc_attach();
    if (!game) {
        perror("Pripojenie k shared memory zlyhalo");
        exit(1);
    }

    // ===== Inicializácia hry =====
    game_init(game, MODE_STANDARD, 0, WORLD_NO_OBSTACLES, NULL);
    game->running = 1;
    game->shutdown = 0;

    // Inicializácia ovocia ako "neaktívne"
    for (int i = 0; i < MAX_FRUITS; i++) {
        game->fruits[i].x = -1;
        game->fruits[i].y = -1;
    }

    // ===== Čakanie na pripojenie hráčov =====
    printf("Cakam na pripojenie hracov...\n");
    int connected = 0;
    while (connected == 0) {
        sem_wait(game_sem);
        connected = 0;
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (game->snakes[i].active)
                connected++;
        }
        sem_post(game_sem);
        usleep(100000); // 100ms
    }
    printf("%d hraci pripojeni. Spustam hru.\n", connected);

    int spawn_counter = 0; // počítadlo ticov pre spawn ovocia

    // ===== Hlavný herný loop =====
    while (1) {
        sem_wait(game_sem);

        // Pohyb hadikov a kontrola kolízií
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

        // ===== Spawn ovocia každých 10 sekúnd, iba 1 fruit =====
        spawn_counter++;
        if (spawn_counter >= FRUIT_SPAWN_INTERVAL) {
            for (int i = 0; i < MAX_FRUITS; i++) {
                if (game->fruits[i].x == -1 && game->fruits[i].y == -1) {
                    game->fruits[i].x = rand() % MAP_W;
                    game->fruits[i].y = rand() % MAP_H;
                    break; // spawn iba 1 fruit
                }
            }
            spawn_counter = 0; // reset counter
        }

        // Čas hry
        game->game_time++;

        // Spočítanie aktívnych hráčov
        int active_players = 0;
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (game->snakes[i].active)
                active_players++;
        }

        // Ukončenie hry, ak všetci zomrú
        if (active_players == 0) {
            printf("Vsetci hraci zomreli. Ukoncenie hry.\n");
            game->running = 0;   // klienti ukončia hlavný loop
            game->shutdown = 1;  // signal pre input vlákna klientov
            sem_post(game_sem);
            break;
        }

        sem_post(game_sem);
        usleep(200000); // 200ms medzi krokmi
    }

    // ===== Počkame, aby sa klienti stihli odmapovať =====
    sleep(1);

    // ===== Cleanup =====
    ipc_destroy(); // zmaže shared memory a semafor
    printf("Server ukoncil hru a uvolnil zdroje.\n");

    return 0;
}