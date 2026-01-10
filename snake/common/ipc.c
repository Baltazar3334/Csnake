#include "ipc.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <semaphore.h>
#include <string.h>

sem_t *game_sem = NULL;

/* ===== SERVER ===== */
int ipc_create(const char* server_name) {
    char shm_name[256];
    char sem_name[256];
    snprintf(shm_name, sizeof(shm_name), "/snake_game_%s", server_name);
    snprintf(sem_name, sizeof(sem_name), "/snake_sem_%s", server_name);

    int fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open");
        return -1;
    }

    if (ftruncate(fd, sizeof(SharedGame)) == -1) {
        perror("ftruncate");
        close(fd);
        return -1;
    }

    SharedGame *game = mmap(NULL, sizeof(SharedGame),
                            PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd, 0);
    if (game == MAP_FAILED) {
        perror("mmap server");
        close(fd);
        return -1;
    }

    // Nastavíme magic ID a vyčistíme shared memory
    memset(game, 0, sizeof(SharedGame));
    game->magic = MAGIC_ID;

    // Vytvorenie semaforu
    game_sem = sem_open(sem_name, O_CREAT | O_EXCL, 0666, 1);
    if (game_sem == SEM_FAILED) {
        if (errno == EEXIST) {
            // semafor už existuje, pripoj sa
            game_sem = sem_open(sem_name, 0);
            if (game_sem == SEM_FAILED) {
                perror("sem_open existujuce");
                munmap(game, sizeof(SharedGame));
                close(fd);
                return -1;
            }
        } else {
            perror("sem_open");
            munmap(game, sizeof(SharedGame));
            close(fd);
            return -1;
        }
    }

    munmap(game, sizeof(SharedGame));
    close(fd); // fd už nepotrebujeme po ftruncate
    return 0;
}

/* ===== CLIENT ===== */
SharedGame* ipc_attach(const char* server_name) {
    char shm_name[256];
    char sem_name[256];
    snprintf(shm_name, sizeof(shm_name), "/snake_game_%s", server_name);
    snprintf(sem_name, sizeof(sem_name), "/snake_sem_%s", server_name);

    int fd = shm_open(shm_name, O_RDWR, 0666);
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

    close(fd);

    // overíme magic ID
    if (ptr->magic != MAGIC_ID) {
        fprintf(stderr, "Shared memory nepatri tomuto serveru!\n");
        munmap(ptr, sizeof(SharedGame));
        return NULL;
    }

    // Pripojenie na existujúci semafor
    game_sem = sem_open(sem_name, 0);
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
void ipc_destroy(const char* server_name) {
    char shm_name[256];
    char sem_name[256];
    snprintf(shm_name, sizeof(shm_name), "/snake_game_%s", server_name);
    snprintf(sem_name, sizeof(sem_name), "/snake_sem_%s", server_name);

    shm_unlink(shm_name);
    sem_unlink(sem_name);
}