#include "ipc.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#define SHM_NAME "/snake_game"
#define SEM_NAME "/snake_sem"

sem_t *game_sem;

int ipc_create() {
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open");
        return -1;
    }

    if (ftruncate(fd, sizeof(SharedGame)) == -1) {
        perror("ftruncate");
        return -1;
    }

    game_sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    if (game_sem == SEM_FAILED) {
        perror("sem_open");
        return -1;
    }

    return fd;
}

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
        return NULL;
    }

    return ptr;
}

void ipc_destroy() {
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_NAME);
}