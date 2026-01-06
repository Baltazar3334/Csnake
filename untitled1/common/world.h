#ifndef WORLD_H
#define WORLD_H

#define MAX_W 100
#define MAX_H 100

typedef enum {
    WORLD_EMPTY,
    WORLD_WALL
} Cell;

typedef struct {
    int width;
    int height;
    Cell grid[MAX_H][MAX_W];
} World;

int world_load(World *w, const char *filename);
int world_is_wall(World *w, int x, int y);

#endif