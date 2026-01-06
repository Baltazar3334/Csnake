#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "../common/ipc.h"
#include "../client/input.h"
#include "../client/render_text.h"

SharedGame *game = NULL;
int my_id = -1; // ID hadíka priradené klientom

void assign_player() {
    sem_wait(game_sem);
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!game->snakes[i].active) {
            game->snakes[i].active = 1;
            game->snakes[i].paused = 0;
            game->snakes[i].dir = DIR_RIGHT;
            game->snakes[i].length = 3;
            game->snakes[i].score = 0;
            my_id = i;
            break;
        }
    }
    sem_post(game_sem);

    if (my_id == -1) {
        printf("Vsetky miesta pre hracov su obsadene!\n");
        exit(1);
    }
}

int main(void) {
    game = ipc_attach();
    if (!game) {
        perror("Pripojenie k serveru zlyhalo");
        exit(1);
    }

    assign_player();

    pthread_t tid;
    pthread_create(&tid, NULL, input_loop, &my_id);

    render_init_text();

    while (game->running && game->snakes[my_id].active) {
        sem_wait(game_sem);
        render_game_text(game, my_id);
        sem_post(game_sem);
        usleep(200000);
    }

    render_cleanup_text();

    printf("Hra skončila.\n");
    return 0;
}

