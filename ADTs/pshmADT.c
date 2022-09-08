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
    const char *shm_name;
    const char *sem_name;
    sem_t *sem;
    char *addr;
    int write_off;
    int read_off;
    fd_t fd;
    size_t size;
    size_t counter_pos;
} pshmCDT;

static int decCounter(pshmADT pshm);
static int incCounter(pshmADT pshm);

// TODO: SEMAPHORES
pshmADT newPshm(const char *shm_name, const char *sem_name, int oflag, mode_t mode) {

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
    new->sem_name = sem_name;

    new->fd = shm_open(new->shm_name, oflag, mode);
    if (new->fd == -1) {
        free(new);
        return NULL;
    }

    new->write_off = 0;
    new->read_off = 0;
    new->size = MAX_PSM_SIZE;
    new->counter_pos = new->size;

    if (oflag & O_CREAT) {
        // If the user wants to create it, use ftruncate
        if (ftruncate(new->fd, new->size * sizeof(char)) == -1) {
            freePshm(new);
            return NULL;
        }
        new->sem = sem_open(new->sem_name, O_CREAT, mode, 0);
        if (new->sem == SEM_FAILED) {
            freePshm(new);
            return NULL;
        }
    } else {
        new->sem = sem_open(new->sem_name, 0);
        if (new->sem == SEM_FAILED) {
            freePshm(new);
            return NULL;
        }
    }

    int prot = PROT_READ; // default
    if (oflag & O_RDWR) {
        prot = prot | PROT_WRITE;
    }

    new->addr = mmap(NULL, new->size * sizeof(char), prot, MAP_SHARED, new->fd, 0);
    if (new->addr == (void *)-1) {
        freePshm(new);
        return NULL;
    }

    // leave space for a current users counter
    new->size--;
    incCounter(new);

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

    sem_post(pshm->sem);

    return bytes_writen;
}

// Reads a line from the shared memory
size_t readPshm(pshmADT pshm, char *buff, size_t bytes) {
    if (pshm == NULL) {
        errno = EINVAL;
        return -1;
    }

    sem_wait(pshm->sem);
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
    int remaining_users = decCounter(pshm);

    munmap(pshm->addr, pshm->size);
    close(pshm->fd);
    if (remaining_users == 0) {
        // CHECK: shm_unlink (only last user should close it)
        // maybe solvabe using a counter in the actual memory space
        shm_unlink(pshm->shm_name);
        sem_unlink(pshm->sem_name);
    }
    sem_close(pshm->sem);
    free(pshm);
    return;
}

// TODO: SEMAPHORES ???
static int decCounter(pshmADT pshm) {
    int qty_users = pshm->addr[pshm->counter_pos]--;
    return qty_users;
}

// TODO: SEMAPHORES ???
static int incCounter(pshmADT pshm) {
    int qty_users = pshm->addr[pshm->counter_pos]++;
    return qty_users;
}

