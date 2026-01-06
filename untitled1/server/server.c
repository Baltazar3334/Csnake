#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

#include "../common/ipc.h"
#include "../common/game.h"

void* game_loop(void* arg) {
    SharedGame *game = arg;

    while (game->running) {
        sem_wait(game_sem);

        /* === ČASOVÝ REŽIM === */
        if (game->mode == MODE_TIME &&
            game->game_time >= game->max_time) {

            printf("Cas hry vyprsal\n");

            for (int i = 0; i < MAX_PLAYERS; i++) {
                game->snakes[i].active = 0;
            }

            game->running = 0;
            sem_post(game_sem);
            break;
            }

        for (int i = 0; i < MAX_PLAYERS; i++) {
            Snake *s = &game->snakes[i];
            if (!s->active || s->paused) continue;

            if (s->pause_timer > 0) {
                s->pause_timer--;
                continue;
            }

            game_move_snake(game, i);

            if (game_check_collision(game, i)) {
                s->active = 0;
            }
        }

        game->game_time++;
        sem_post(game_sem);
        sleep(1);
    }
    return NULL;
}

int main(int argc, char **argv) {
    ipc_create();
    SharedGame *game = ipc_attach();

    // napr. časový režim na 60 sekúnd
    game_init(game, MODE_TIME, 60);

    pthread_t loop;
    pthread_create(&loop, NULL, game_loop, game);

    pthread_join(loop, NULL);
    ipc_destroy();
    return 0;
}