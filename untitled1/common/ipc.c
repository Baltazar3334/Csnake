#include "ipc.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#define SHM_NAME "/snake_game"
#define SEM_NAME "/snake_sem"

sem_t *game_sem;

int ipc_create() {
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(SharedGame));

    game_sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    return fd;
}

SharedGame* ipc_attach() {
    int fd = shm_open(SHM_NAME, O_RDWR, 0666);
    return mmap(NULL, sizeof(SharedGame),
                PROT_READ | PROT_WRITE,
                MAP_SHARED, fd, 0);
}

void ipc_destroy() {
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_NAME);
}