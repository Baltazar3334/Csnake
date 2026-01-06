#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "../common/config.h"
#include <termios.h>
#include <fcntl.h>
#include "../common/ipc.h"

extern SharedGame *game; // pre prístup k shared state

// nastavenie terminalu na non-blocking vstup
static void set_input_nonblocking() {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}

// reset terminalu pri ukonceni
static void reset_input() {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

void* input_loop(void *arg) {
    int id = *(int*)arg;
    set_input_nonblocking();

    while (1) {
        char ch;
        ssize_t r = read(STDIN_FILENO, &ch, 1);
        if (r <= 0) { usleep(50000); continue; }

        sem_wait(game_sem);

        Snake *s = &game->snakes[id];
        if (!s->active) { sem_post(game_sem); break; }

        switch (ch) {
            case 'w': if (s->dir != DIR_DOWN) s->dir = DIR_UP; break;
            case 's': if (s->dir != DIR_UP) s->dir = DIR_DOWN; break;
            case 'a': if (s->dir != DIR_RIGHT) s->dir = DIR_LEFT; break;
            case 'd': if (s->dir != DIR_LEFT) s->dir = DIR_RIGHT; break;
            case 'p': s->paused = 1; break;
            case 'r': if (s->paused) { s->paused = 0; s->pause_timer = 3; } break;
            case 'q': s->active = 0; break;
        }

        sem_post(game_sem);
        usleep(50000);
    }

    reset_input();
    return NULL;
}