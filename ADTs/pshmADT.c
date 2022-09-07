#include "pshmADT.h"
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#define MAX_PSM_SIZE INT_MAX

typedef int fd_t;

typedef struct pshmCDT {
    // char name[MAX_NAME_LEN];
    const char * shm_name;
    const char * sem_name;
    sem_t * sem;
    char * addr;
    int write_off;
    int read_off;
    fd_t fd;
    size_t size;
    size_t counter_pos;
} pshmCDT;

static int decCounter(pshmADT pshm);
static int incCounter(pshmADT pshm);

// TODO: SEMAPHORES
pshmADT newPshm(const char *shm_name, const char * sem_name, int oflag, mode_t mode) {
    if(shm_name == NULL) {
        errno = EINVAL; // Invalid argument
        return NULL;
    }
    pshmADT new = malloc(sizeof(pshmCDT));
    if(new == NULL) {
        return NULL;
    }

    // TODO: CHECK FAILURE
    //strncpy(new->shm_name, name, MAX_NAME_LEN - 1);
    new->shm_name = shm_name;
    new->sem_name = sem_name;

    new->fd = shm_open(new->shm_name, oflag, mode);
    if(new->fd == -1) {
        free(new);
        return NULL;
    }

    new->write_off = 0;
    new->read_off = 0;
    new->size = MAX_PSM_SIZE;
    new->counter_pos = new->size;

    if(oflag & O_CREAT) {
        // If the user wants to create it, use ftruncate
        if(ftruncate(new->fd, new->size*sizeof(char)) == -1) {
            freePshm(new);
            return NULL;
        }
        new->sem = sem_open(new->sem_name, O_CREAT, mode, 0);
        if(new->sem == SEM_FAILED) {
            freePshm(new);
            return NULL;
        }
    } else {
        new->sem = sem_open(new->sem_name, 0);
        if(new->sem == SEM_FAILED) {
            freePshm(new);
            return NULL;
        }
    }

    int prot = PROT_READ; // default
    if(oflag & O_RDWR) {
        prot = prot | PROT_WRITE;
    }

    new->addr = mmap(NULL, new->size*sizeof(char), prot, MAP_SHARED, new->fd, 0);
    if(new->addr == (void *) -1) {
        freePshm(new);
        return NULL;
    }


    // leave space for a current users counter
    new->size--;
    incCounter(new);

    return new;
}

// TODO: SEMAPHORES
size_t writePshm(pshmADT pshm, const char * buff, size_t bytes) {
    if(pshm == NULL) {
        errno = EINVAL;
        return 0;
    }
    ssize_t bytes_writen = 0;

    // TODO: write & semaphores

    return bytes_writen;
}

// TODO: SEMAPHORES
size_t readPshm(pshmADT pshm, char * buff, size_t bytes) {
    if(pshm == NULL) {
        errno = EINVAL;
        return 0;
    }

    ssize_t bytes_read = 0;

    // TODO: read & semaphores

    return bytes_read;
}

void freePshm(pshmADT pshm) {
    if(pshm == NULL) {
        errno = EINVAL;
        return;
    }
    int remaining_users = decCounter(pshm);

    munmap(pshm->addr, pshm->size);
    close(pshm->fd);
    if(remaining_users == 0) {
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
