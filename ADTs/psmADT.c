#include "psmADT.h"
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#define MAX_PSM_SIZE INT_MAX

typedef int fd_t;

typedef struct psmCDT {
    // char name[MAX_NAME_LEN];
    const char * name;
    char * addr;
    int write_off;
    int read_off;
    fd_t fd;
    size_t size;
    size_t counter_pos;
} psmCDT;

static int decCounter(psmADT psm);
static int incCounter(psmADT psm);

// TODO: SEMAPHORES
psmADT newPsm(const char *name, int oflag, mode_t mode) {
    if(name == NULL) {
        errno = EINVAL; // Invalid argument
        return NULL;
    }
    psmADT new = malloc(sizeof(psmCDT));
    if(new == NULL) {
        return NULL;
    }

    // TODO: CHECK FAILURE
    //strncpy(new->name, name, MAX_NAME_LEN - 1);
    new->name = name;

    new->fd = shm_open(new->name, oflag, mode);
    if(new->fd == -1) {
        free(new);
        return NULL;
    }

    new->write_off = 0;
    new->read_off = 0;
    new->size = MAX_PSM_SIZE;
    new->counter_pos = new->size;

    // If the user wants to create it, use ftruncate
    if((oflag & O_CREAT) && ftruncate(new->fd, new->size*sizeof(char)) == -1) {
        freePsm(new);
        return NULL;
    }

    int prot = PROT_READ; // default
    if(oflag & O_RDWR) {
        prot = prot | PROT_WRITE;
    }

    new->addr = mmap(NULL, new->size*sizeof(char), prot, MAP_SHARED, new->fd, 0);
    if(new->addr == (void *) -1) {
        freePsm(new);
        return NULL;
    }

    // leave space for a current users counter
    new->size--;

    incCounter(new);

    return new;
}

// TODO: SEMAPHORES
size_t writePsm(psmADT psm, const char * buff, size_t bytes) {
    if(psm == NULL) {
        errno = EINVAL;
        return 0;
    }
    ssize_t bytes_writen = 0;

    // TODO: write & semaphores

    return bytes_writen;
}

// TODO: SEMAPHORES
size_t readPsm(psmADT psm, char * buff, size_t bytes) {
    if(psm == NULL) {
        errno = EINVAL;
        return 0;
    }

    ssize_t bytes_read = 0;

    // TODO: read & semaphores

    return bytes_read;
}

void freePsm(psmADT psm) {
    if(psm == NULL) {
        errno = EINVAL;
        return;
    }
    int remaining_users = decCounter(psm);

    munmap(psm->addr, psm->size);
    close(psm->fd);
    if(remaining_users == 0) {
        // CHECK: shm_unlink (only last user should close it)
        // maybe solvabe using a counter in the actual memory space
        shm_unlink(psm->name);
    }
    free(psm);
    return;
}

// TODO: SEMAPHORES ???
static int decCounter(psmADT psm) {
    int qty_users = psm->addr[psm->counter_pos]--;
    return qty_users;
}

// TODO: SEMAPHORES ???
static int incCounter(psmADT psm) {
    int qty_users = psm->addr[psm->counter_pos]++;
    return qty_users;
}
