#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "../common/game.h"
#include "../common/ipc.h"
#include "../common/world.h"
#include "input.h"
#include "render_text.h"



SharedGame *game; // globálna pre input.c

int main() {
    game = ipc_attach();
    if (!game) { perror("ipc_attach"); return 1; }

    // kontrola, či hra beží
    sem_wait(game_sem);
    if (!game->running) {
        sem_post(game_sem);
        printf("Hra je uz ukoncena – neda sa pripojit\n");
        return 0;
    }
    sem_post(game_sem);

    // registrácia hráča
    sem_wait(game_sem);
    int id = -1;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!game->snakes[i].active) {
            id = i;
            Snake *s = &game->snakes[i];
            s->active = 1; s->paused = 0; s->pause_timer = 0;
            s->score = 0; s->length = 3; s->dir = DIR_RIGHT;
            for (int j = 0; j < s->length; j++) {
                s->body[j].x = 10 - j; s->body[j].y = 10;
            }
            break;
        }
    }
    sem_post(game_sem);

    if (id == -1) { printf("Hra je plná\n"); return 0; }

    // inicializácia textového renderu
    render_init_text();

    // spustenie vstupného vlákna
    pthread_t input_thread;
    pthread_create(&input_thread, NULL, input_loop, &id);

    // hlavný cyklus
    while (1) {
        sem_wait(game_sem);
        Snake *s = &game->snakes[id];
        if (!s->active) { sem_post(game_sem); break; }

        render_game_text(game, id);

        sem_post(game_sem);
        usleep(200000);
    }

    pthread_join(input_thread, NULL);

    render_cleanup_text();

    printf("\nKoniec hry\n");
    return 0;
}

