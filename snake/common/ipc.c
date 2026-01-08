#include "ipc.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#define SHM_NAME "/snake_game"
#define SEM_NAME "/snake_sem"

sem_t *game_sem = NULL;

/* ===== SERVER ===== */
int ipc_create() {
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open");
        return -1;
    }

    if (ftruncate(fd, sizeof(SharedGame)) == -1) {
        perror("ftruncate");
        close(fd);
        return -1;
    }

    /* pokus vytvoriť semafor */
    game_sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0666, 1);
    if (game_sem == SEM_FAILED) {
        if (errno == EEXIST) {
            /* už existuje → pripoj sa */
            game_sem = sem_open(SEM_NAME, 0);
        }
    }

    if (game_sem == SEM_FAILED) {
        perror("sem_open");
        close(fd);
        return -1;
    }

    return fd;
}

/* ===== CLIENT ===== */
SharedGame* ipc_attach() {
    int fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open attach");
        return NULL;
    }

    SharedGame *ptr = mmap(NULL, sizeof(SharedGame),
                           PROT_READ | PROT_WRITE,
                           MAP_SHARED, fd, 0);

    if (ptr == MAP_FAILED) {
        perror("mmap attach");
        close(fd);
        return NULL;
    }

    /* pripojenie na existujúci semafor */
    game_sem = sem_open(SEM_NAME, 0);
    if (game_sem == SEM_FAILED) {
        perror("sem_open attach");
        return NULL;
    }

    return ptr;
}

/* ===== CLEANUP ===== */
void ipc_destroy() {
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_NAME);
}