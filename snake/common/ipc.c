#include "ipc.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#define SHM_NAME "/snake_game_baltazar"
#define SEM_NAME "/snake_sem_baltazar"

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

    // Vytvorenie semaforu
    game_sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0666, 1);
    if (game_sem == SEM_FAILED) {
        if (errno == EEXIST) {
            // už existuje → pripoj sa
            game_sem = sem_open(SEM_NAME, 0);
            if (game_sem == SEM_FAILED) {
                perror("sem_open existujuce");
                close(fd);
                return -1;
            }
        } else {
            perror("sem_open");
            close(fd);
            return -1;
        }
    }

    close(fd); // fd už nepotrebujeme po ftruncate
    return 0;
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

    close(fd); // fd už nepotrebujeme po mmap

    // Pripojenie na existujúci semafor
    game_sem = sem_open(SEM_NAME, 0);
    if (game_sem == SEM_FAILED) {
        perror("sem_open attach");
        munmap(ptr, sizeof(SharedGame));
        return NULL;
    }

    return ptr;
}

/* ===== CLEANUP PRE KLIENTA ===== */
void ipc_detach(SharedGame *ptr) {
    if (ptr) {
        munmap(ptr, sizeof(SharedGame));
    }
    if (game_sem) {
        sem_close(game_sem);
        game_sem = NULL;
    }
}

/* ===== CLEANUP PRE SERVER ===== */
void ipc_destroy() {
    // Odstráni shared memory a semafor
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_NAME);
}