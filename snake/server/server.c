#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#include "../common/ipc.h"
#include "../common/game.h"

int main(void) {
    // ===== 1️⃣ Vytvorenie shared memory a semaforu =====
    if (ipc_create() == -1) {
        fprintf(stderr, "Nepodarilo sa vytvorit shared memory a semafor\n");
        exit(1);
    }

    // ===== 2️⃣ Pripojenie servera k shared memory =====
    SharedGame *game = ipc_attach();
    if (!game) {
        perror("Pripojenie k shared memory zlyhalo");
        exit(1);
    }

    // ===== 3️⃣ Inicializácia hry =====
    game_init(game, MODE_STANDARD, 0, WORLD_NO_OBSTACLES, NULL);
    game->running = 1;
    game->shutdown = 0;

    // ===== 4️⃣ Čakanie na pripojenie hráčov =====
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

    // ===== 5️⃣ Hlavný herný loop =====
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

        // Spawn ovocia
        game_spawn_fruits(game);

        // Čas hry
        game->game_time++;

        // Spočítanie aktívnych hráčov po pohybe
        int active_players = 0;
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (game->snakes[i].active)
                active_players++;
        }

        // Ak už nie sú aktívni hráči, ukonči hru
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

    // ===== 6️⃣ Počkame, aby sa klienti stihli odmapovať =====
    sleep(1);

    // ===== 7️⃣ Cleanup =====
    ipc_destroy(); // zmaže shared memory a semafor
    printf("Server ukoncil hru a uvolnil zdroje.\n");

    return 0;
}