#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "../common/ipc.h"
#include "../client/input.h"
#include "../client/render_text.h"
#include "../common/world.h"

SharedGame *game = NULL;
int my_id = -1; // ID hadíka priradené klientom

void assign_player() {
    sem_wait(game_sem);
    for (int i = 0; i < MAX_PLAYERS; i++) {
        Snake *s = &game->snakes[i];
        if (!s->active) {
            int x, y;
            int found = 0;

            // bezpecne miesto na spawn
            for (int attempt = 0; attempt < 1000; attempt++) {
                x = rand() % game->world.width;
                y = rand() % game->world.height;

                if (world_is_safe_spawn(&game->world, x, y)) {
                    found = 1;
                    break;
                }
            }

            if (!found) {
                sem_post(game_sem);
                printf("Nenaslo sa bezpecne miesto pre hada!\n");
                exit(1);
            }

            s->active = 1;
            s->paused = 0;
            s->dir = DIR_RIGHT;
            s->length = 3;
            s->score = 0;

            // HLAVA
            s->body[0].x = x;
            s->body[0].y = y;

            // TELO (za hlavou)
            for (int j = 1; j < s->length; j++) {
                s->body[j].x = x - j;
                s->body[j].y = y;
            }

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

    // ===== Input thread =====
    int *id_copy = malloc(sizeof(int));
    *id_copy = my_id;

    pthread_t tid;
    pthread_create(&tid, NULL, input_loop, id_copy);

    render_init_text();

    // ===== HLAVNÝ RENDER LOOP =====
    while (game->running && game->snakes[my_id].active) {
        sem_wait(game_sem);
        render_game_text(game, my_id);
        sem_post(game_sem);
        usleep(100000);
    }

    // ===== OBRAZOVKA PO SMRTI =====
    sem_wait(game_sem);

    int final_score = game->snakes[my_id].score;
    int server_shutdown = game->shutdown;

    sem_post(game_sem);

    render_cleanup_text();
    printf("\033[2J\033[H"); // vyčistenie terminálu

    if (!server_shutdown) {
        printf("\n");
        printf("******************************\n");
        printf("*                            *\n");
        printf("*        Z O M R E L  S I     *\n");
        printf("*                            *\n");
        printf("*   Skore: %-6d            *\n", final_score);
        printf("*                            *\n");
        printf("******************************\n");
        printf("\n");
        printf("Stlac ENTER pre ukoncenie...\n");
        getchar();
    }

    // ===== UKONČENIE =====
    pthread_join(tid, NULL);
    free(id_copy);

    ipc_detach(game);

    printf("Klient ukoncil hru.\n");
    return 0;
}

