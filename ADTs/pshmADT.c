// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "pshmADT.h"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_PSM_SIZE INT_MAX
#define END_IDENTIFIER '\n'

typedef int fd_t;

typedef struct pshmCDT {
    // char name[MAX_NAME_LEN];
    const char * shm_name;
    const char * sem_rw_name;
    const char * sem_connected_name;
    sem_t * sem_rw;
    sem_t * sem_connected;
    char * addr;
    int write_off;
    int read_off;
    fd_t fd;
    size_t size;
} pshmCDT;

#define SEM_RW "/SEM_RW"
#define SEM_CONNECTED "/SEM_CONECTED"
#define SHM_ERROR -1

// TODO: SEMAPHORES
pshmADT newPshm(const char *shm_name, int oflag, mode_t mode) {
    if (shm_name == NULL) {
        errno = EINVAL; // Invalid argument
        return NULL;
    }
    pshmADT new = malloc(sizeof(pshmCDT));
    if (new == NULL) {
        return NULL;
    }

    // TODO: CHECK FAILURE
    // strncpy(new->shm_name, name, MAX_NAME_LEN - 1);
    new->shm_name = shm_name;
    new->sem_rw_name = SEM_RW;
    new->sem_connected_name = SEM_CONNECTED;

    new->fd = shm_open(new->shm_name, oflag, mode);
    if (new->fd == SHM_ERROR) {
        perror("shm_open");
        free(new);
        return NULL;
    }

    new->write_off = 0;
    new->read_off = 0;
    new->size = MAX_PSM_SIZE;

    if (oflag & O_CREAT) {
        // If the user wants to create it, use ftruncate
        if (ftruncate(new->fd, new->size * sizeof(char)) == SHM_ERROR) {
            perror("ftruncate");
            freePshm(new);
            return NULL;
        }
    }

    new->sem_rw = sem_open(new->sem_rw_name, O_CREAT, mode, 0);
    if (new->sem_rw == SEM_FAILED) {
        perror("sem r/w");
        freePshm(new);
        return NULL;
    }

    new->sem_connected = sem_open(new->sem_connected_name, O_CREAT, mode, 0);
    if(new->sem_connected == SEM_FAILED) {
        perror("sem connected");
        freePshm(new);
        return NULL;
    }
    // notify that im connected
    sem_post(new->sem_connected);

    int prot = PROT_READ; // default
    if (oflag & O_RDWR) {
        prot = prot | PROT_WRITE;
    }

    new->addr = mmap(NULL, new->size * sizeof(char), prot, MAP_SHARED, new->fd, 0);
    if (new->addr == (void *) SHM_ERROR) {
        perror("mmap");
        freePshm(new);
        return NULL;
    }

    return new;
}

// Writes a line to the shared memory
size_t writePshm(pshmADT pshm, const char *buff, size_t bytes) {
    if (pshm == NULL) {
        errno = EINVAL;
        return -1;
    }

    // Critical section
    ssize_t bytes_writen;
    for (bytes_writen = 0; bytes_writen < bytes; bytes_writen++) {

        // Stop writing if string ends
        if (buff[bytes_writen] == '\0') {
            break;
        }

        pshm->addr[pshm->write_off] = buff[bytes_writen];
        pshm->write_off = (pshm->write_off + 1) % pshm->size; // circular buffer

        // Stop writing at end of line
        if (buff[bytes_writen] == END_IDENTIFIER) {
            break;
        }
    }

    sem_post(pshm->sem_rw);

    return bytes_writen;
}

// Reads a line from the shared memory
size_t readPshm(pshmADT pshm, char *buff, size_t bytes) {
    if (pshm == NULL) {
        errno = EINVAL;
        return -1;
    }

    sem_wait(pshm->sem_rw);
    // Critical section
    ssize_t bytes_read;
    // Read byte for byte until bytes is reached, if byte equals END_IDENTIFIER,
    // stop reading
    for (bytes_read = 0; bytes_read < bytes - 1; bytes_read++) {
        buff[bytes_read] = pshm->addr[pshm->read_off];
        pshm->read_off = (pshm->read_off + 1) % pshm->size; // circular buffer

        if (pshm->addr[pshm->read_off] == END_IDENTIFIER) {
            break;
        }
    }

    buff[bytes_read] = '\0';
    return bytes_read + 1;
}

void freePshm(pshmADT pshm) {
    if (pshm == NULL) {
        errno = EINVAL;
        return;
    }
    sem_trywait(pshm->sem_connected); // non blocking decrement
    int remaining_users = 0;
    sem_getvalue(pshm->sem_connected, &remaining_users);
    munmap(pshm->addr, pshm->size);
    close(pshm->fd);
    if (remaining_users == 0) {
        puts("LEAVING... <= REMOVE THIS PRINT AFTER");
        // CHECK: shm_unlink (only last user should close it)
        // maybe solvabe using a counter in the actual memory space
        shm_unlink(pshm->shm_name);
        sem_unlink(pshm->sem_rw_name);
        sem_unlink(pshm->sem_connected_name);
    }
    sem_close(pshm->sem_rw);
    sem_close(pshm->sem_connected);
    free(pshm);
    return;
}
