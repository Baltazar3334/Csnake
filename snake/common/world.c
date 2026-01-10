#include "../common/config.h"
#include "world.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int world_load(World *w, const char *filename) {
    char path[512];

    if (strstr(filename, "/") || strstr(filename, "..")) {
        printf("Neplatny nazov mapy, piste len meno nie cestu\n");
        return 0;
    }

    // citanie zo suboru
    if (strstr(filename, ".txt")) {
        snprintf(path, sizeof(path), "../snake/maps/%s", filename);
    } else {
        snprintf(path, sizeof(path), "../snake/maps/%s.txt", filename);
    }

    FILE *f = fopen(path, "r");
    if (!f) {
        printf("Subor '%s' sa nenasiel\n", path);
        return 0;
    }

    char line[MAX_W + 2];
    int h = 0;

    while (fgets(line, sizeof(line), f)) {
        int len = strlen(line);
        if (line[len - 1] == '\n') line[len - 1] = 0;

        w->width = strlen(line);
        for (int x = 0; x < w->width; x++) {
            w->grid[h][x] =
                (line[x] == '#') ? WORLD_WALL : WORLD_EMPTY;
        }
        h++;
    }

    w->height = h;
    fclose(f);
    return 1;
}

int world_is_safe_spawn(World *w, int x, int y) {
    // kontrola okolia 3x3 ci je bezpecne na spawn hadika
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx;
            int ny = y + dy;

            if (world_is_wall(w, nx, ny))
                return 0;
        }
    }
    return 1;
}

int world_is_wall(World *w, int x, int y) {
    if (x < 0 || y < 0 || x >= w->width || y >= w->height)
        return 1;
    return w->grid[y][x] == WORLD_WALL;
}

void world_random_generate(World *w) {
    w->width = MAP_W;
    w->height = MAP_H;

    // najprv všetko voľné
    memset(w->cells, 0, sizeof(w->cells));

    // vložíme pevné prekážky, napr. 20% mapy
    int total_cells = w->width * w->height;
    int obstacles = total_cells / 25;

    for (int i = 0; i < obstacles; i++) {
        int x, y;
        do {
            x = rand() % w->width;
            y = rand() % w->height;
        } while (w->cells[y][x] != 0); // už je obsadené

        w->cells[y][x] = 1; // 1 = prekážka
    }

    // overíme, že všetky prístupné polia sú dosiahnuteľné
    int visited[MAP_H][MAP_W];
    memset(visited, 0, sizeof(visited));

    int start_x = 0, start_y = 0;
    while (w->cells[start_y][start_x] == 1) { // hľadáme voľné políčko
        start_x = rand() % w->width;
        start_y = rand() % w->height;
    }

    flood_fill(w, visited, start_x, start_y);

    // všetky neprístupné polia označíme ako prekážky
    for (int y = 0; y < w->height; y++) {
        for (int x = 0; x < w->width; x++) {
            if (!visited[y][x] && w->cells[y][x] == 0) {
                w->cells[y][x] = 1;
            }
        }
    }


    for (int y = 0; y < w->height; y++) {
        for (int x = 0; x < w->width; x++) {
            w->grid[y][x] = (w->cells[y][x] == 1) ? WORLD_WALL : WORLD_EMPTY;
        }
    }
}

// Flood-fill implementácia
void flood_fill(World *w, int visited[MAP_H][MAP_W], int x, int y) {
    if (x < 0 || x >= w->width || y < 0 || y >= w->height) return;
    if (w->cells[y][x] == 1 || visited[y][x]) return;

    visited[y][x] = 1;

    flood_fill(w, visited, x + 1, y);
    flood_fill(w, visited, x - 1, y);
    flood_fill(w, visited, x, y + 1);
    flood_fill(w, visited, x, y - 1);
}