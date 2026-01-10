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
    int cells[MAP_H][MAP_W]; // 0 = voľné, 1 = prekážka
} World;

void world_random_generate(World *w);
void flood_fill(World *w, int visited[MAP_H][MAP_W], int x, int y);
int world_load(World *w, const char *filename);
int world_is_wall(World *w, int x, int y);
int world_is_safe_spawn(World *w, int x, int y);

#endif