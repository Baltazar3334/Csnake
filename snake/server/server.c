#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#include "../common/ipc.h"
#include "../common/game.h"

#define FRUIT_SPAWN_INTERVAL 50 // každých 50 ticov = ~10 sekúnd
#define TICK_USEC 200000          // 200 ms
#define NO_PLAYER_TIMEOUT 50      // 50 tickov = ~10 sekúnd

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

    // vyber rezimu hry
    int choice = 0;
    int max_time_seconds = 0;

    printf("Vyber herny rezim:\n");
    printf("1 - Standardny (10s bez hracov)\n");
    printf("2 - Casovy\n");
    printf("Volba: ");
    fflush(stdout);

    scanf("%d", &choice);

    // ===== Inicializácia hry =====
    if (choice == 2) {
        printf("Zadaj dlzku hry v sekundach: ");
        fflush(stdout);
        scanf("%d", &max_time_seconds);

        game_init(game, MODE_TIME, 0, WORLD_NO_OBSTACLES, NULL);

        // 1 tick = 200 ms → 5 tickov = 1 sekunda
        game->max_time = max_time_seconds * 5;

        printf("Spustam CASOVY rezim (%d sekund)\n", max_time_seconds);
    } else {
        game_init(game, MODE_STANDARD, 0, WORLD_NO_OBSTACLES, NULL);
        printf("Spustam STANDARDNY rezim\n");
    }

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
    int no_player_ticks = 0;

    while (1) {
        sem_wait(game_sem);

        int active_players = 0;

        // ===== Pohyb hadíkov =====
        for (int i = 0; i < MAX_PLAYERS; i++) {
            Snake *s = &game->snakes[i];
            if (!s->active || s->paused)
                continue;

            active_players++;

            game_move_snake(game, i);

            if (game_check_collision(game, i)) {
                printf("Hadik %d zomrel.\n", i);
                s->active = 0;
            }
        }

        // ===== Spawn ovocia každých ~10 sekúnd (1 kus) =====
        spawn_counter++;
        if (spawn_counter >= FRUIT_SPAWN_INTERVAL) {
            for (int i = 0; i < MAX_FRUITS; i++) {
                if (game->fruits[i].x == -1 && game->fruits[i].y == -1) {
                    game->fruits[i].x = rand() % MAP_W;
                    game->fruits[i].y = rand() % MAP_H;
                    break;
                }
            }
            spawn_counter = 0;
        }

        // ===== Čas hry =====
        game->game_time++;

        // ===== HERNÉ REŽIMY =====
        if (game->mode == MODE_STANDARD) {

            if (active_players == 0) {
                no_player_ticks++;

                if (no_player_ticks >= NO_PLAYER_TIMEOUT) {
                    printf("10 sekund bez hracov – koniec hry.\n");
                    game->running = 0;
                    game->shutdown = 1;
                    sem_post(game_sem);
                    break;
                }

            } else {
                no_player_ticks = 0; // niekto hrá → reset
            }

        } else if (game->mode == MODE_TIME) {

            if (game->game_time >= game->max_time) {
                printf("Vyprsal herny cas – koniec hry.\n");
                game->running = 0;
                game->shutdown = 1;
                sem_post(game_sem);
                break;
            }
        }

        sem_post(game_sem);
        usleep(TICK_USEC);
    }

    // ===== Počkame, aby sa klienti stihli odmapovať =====
    sleep(1);

    // ===== Cleanup =====
    ipc_destroy(); // zmaže shared memory a semafor
    printf("Server ukoncil hru a uvolnil zdroje.\n");

    return 0;
}