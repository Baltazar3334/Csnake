#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>

#include "../common/ipc.h"
#include "../common/game.h"

#define FRUIT_SPAWN_INTERVAL 50   // 50 tickov ≈ 10 sekúnd
#define TICK_USEC 200000          // 200 ms
#define NO_PLAYER_TIMEOUT 50      // 10 sekúnd bez hráča

// ===== Pomocné funkcie =====

int fruit_at(SharedGame *game, int x, int y) {
    for (int i = 0; i < MAX_FRUITS; i++) {
        if (game->fruits[i].x == x && game->fruits[i].y == y)
            return 1;
    }
    return 0;
}

int snake_at(SharedGame *game, int x, int y) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        Snake *s = &game->snakes[i];
        if (!s->active) continue;

        for (int j = 0; j < s->length; j++) {
            if (s->body[j].x == x && s->body[j].y == y)
                return 1;
        }
    }
    return 0;
}

// ===== Main =====

int main(void) {
    srand(time(NULL));

    if (ipc_create() == -1) {
        fprintf(stderr, "Nepodarilo sa vytvorit shared memory\n");
        exit(1);
    }

    SharedGame *game = ipc_attach();
    if (!game) {
        perror("ipc_attach");
        exit(1);
    }

    sem_wait(game_sem);
    for (int i = 0; i < MAX_FRUITS; i++) {
        game->fruits[i].x = -1;
        game->fruits[i].y = -1;
    }
    sem_post(game_sem);

    int choice = 0;
    int max_time_seconds = 0;

    printf("Vyber herny rezim:\n");
    printf("1 - Standardny (10s bez hracov)\n");
    printf("2 - Casovy\n");
    printf("Volba: ");
    fflush(stdout);
    scanf("%d", &choice);

    // ===== NOVÝ: výber typu sveta =====
    int choice_world = 0;
    char mapfile[256] = {0};
    printf("Vyber typ sveta:\n");
    printf("1 - Svet bez prekazok\n");
    printf("2 - Svet s prekazkami (moze sa nacitat z mapy alebo generovat)\n");
    printf("Volba: ");
    fflush(stdout);
    scanf("%d", &choice_world);

    WorldType wtype = (choice_world == 2) ? WORLD_WITH_OBSTACLES : WORLD_NO_OBSTACLES;

    // ak svet s prekážkami → voliteľne načítanie mapy
    if (wtype == WORLD_WITH_OBSTACLES) {
        printf("Zadaj nazov suboru s mapou (prazdne = generovat nahodne): ");
        fflush(stdout);
        scanf("%s", mapfile);
        if (strlen(mapfile) == 0) {
            mapfile[0] = 0; // nebude nacitavat, bude generovat nahodne
        }
    }

    if (choice == 2) {
        printf("Zadaj dlzku hry v sekundach: ");
        fflush(stdout);
        scanf("%d", &max_time_seconds);

        game_init(game, MODE_TIME, 0, WORLD_NO_OBSTACLES, NULL);
        game->max_time = max_time_seconds * 5; // 5 tickov = 1s
        printf("Spustam CASOVY rezim (%d s)\n", max_time_seconds);
    } else {
        game_init(game, MODE_STANDARD, 0, WORLD_NO_OBSTACLES, NULL);
        printf("Spustam STANDARDNY rezim\n");
    }



    game->running = 1;
    game->shutdown = 0;

    // inicializácia ovocia ako "neaktívne"
    for (int i = 0; i < MAX_FRUITS; i++) {
        game->fruits[i].x = -1;
        game->fruits[i].y = -1;
    }

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
        usleep(100000);
    }

    printf("%d hraci pripojeni. Spustam hru.\n", connected);

    int spawn_counter = 0;
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

        // ===== Spawn ovocia podľa počtu aktívnych hadov každých 10 sekúnd =====
        spawn_counter++;
        if (spawn_counter >= FRUIT_SPAWN_INTERVAL) {

            // spočítajme, koľko ovocia momentálne existuje
            int current_fruits = 0;
            for (int i = 0; i < MAX_FRUITS; i++) {
                if (game->fruits[i].x != -1)
                    current_fruits++;
            }

            // koľko ovocia ešte treba doplniť
            int needed = active_players - current_fruits;

            for (int k = 0; k < needed; k++) {
                for (int attempt = 0; attempt < 100; attempt++) {

                    int x = rand() % MAP_W;
                    int y = rand() % MAP_H;

                    if (snake_at(game, x, y)) continue;
                    if (fruit_at(game, x, y)) continue;

                    // nájdeme prvé prázdne miesto v poli ovocia
                    for (int i = 0; i < MAX_FRUITS; i++) {
                        if (game->fruits[i].x == -1) {
                            game->fruits[i].x = x;
                            game->fruits[i].y = y;
                            break;
                        }
                    }
                    break;
                }
            }

            spawn_counter = 0;
        }

        // ===== Čas hry =====
        game->game_time++;

        // ===== Herné režimy =====
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
                no_player_ticks = 0;
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

    sleep(1);
    ipc_destroy();

    printf("Server ukoncil hru a uvolnil zdroje.\n");
    return 0;
}