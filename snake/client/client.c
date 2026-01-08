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
        Snake *s = &game->snakes[i];
        if (!s->active) {
            s->active = 1;
            s->paused = 0;
            s->dir = DIR_RIGHT;
            s->length = 3;
            s->score = 0;
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

    // bezpečne posielame my_id do vlákna
    int *id_copy = malloc(sizeof(int));
    *id_copy = my_id;

    pthread_t tid;
    pthread_create(&tid, NULL, input_loop, id_copy);

    render_init_text();

    while (game->running && game->snakes[my_id].active) {
        sem_wait(game_sem);
        render_game_text(game, my_id);
        sem_post(game_sem);
        usleep(100000); // 10 FPS, plynulejšie
    }

    render_cleanup_text();

    pthread_join(tid, NULL);
    free(id_copy);

    // bezpečný cleanup
    ipc_detach(game);

    printf("Klient ukoncil hru.\n");
    return 0;
}

