#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../common/ipc.h"

SharedGame *game = NULL;

void render() {
    if (!game) return;

    printf("\033[H\033[J");  // vyčistí obrazovku
    for (int y = 0; y < game->world.height; y++) {
        for (int x = 0; x < game->world.width; x++) {
            char c = '.';
            if (game->world.grid[y][x] == WORLD_WALL)
                c = '#';
            for (int i = 0; i < MAX_PLAYERS; i++) {
                if (!game->snakes[i].active) continue;
                for (int j = 0; j < game->snakes[i].length; j++) {
                    if (game->snakes[i].body[j].x == x &&
                        game->snakes[i].body[j].y == y)
                        c = 'S';
                }
            }
            for (int i = 0; i < MAX_FRUITS; i++) {
                if (game->fruits[i].x == x && game->fruits[i].y == y)
                    c = 'F';
            }
            printf("%c", c);
        }
        printf("\n");
    }
}

int main(void) {
    game = ipc_attach();
    if (!game) {
        perror("Pripojenie k serveru zlyhalo");
        exit(1);
    }

    while (game->running) {
        render();
        usleep(200000);
    }

    printf("Hra skončila.\n");
    return 0;
}

