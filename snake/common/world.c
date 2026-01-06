#include "world.h"
#include <stdio.h>
#include <string.h>

int world_load(World *w, const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return 0;

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

int world_is_wall(World *w, int x, int y) {
    if (x < 0 || y < 0 || x >= w->width || y >= w->height)
        return 1;
    return w->grid[y][x] == WORLD_WALL;
}