#include "psmADT.h"
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

typedef int fd_t;

typedef struct psmCDT {
    size_t size;
    void * addr; // TODO: REPLACE VOID *
    fd_t fd;
    char name[MAX_NAME_LEN];
} psmCDT;


psmADT newPsm(const char *name, int oflag, mode_t mode) {
    psmADT new = malloc(sizeof(psmCDT));
    if(new == NULL) {
        return NULL;
    }

    // TODO: CHECK FAILURE
    strncpy(new->name, name, MAX_NAME_LEN - 1);

    new->fd = shm_open(new->name, oflag, mode);
    if(new->fd == -1) {
        free(new);
        return NULL;
    }

    // TODO: DEFINE PROPER SIZE
    new->size = 4;

    // If the user wants to create it, use ftruncate
    if((oflag & O_CREAT) && ftruncate(new->fd, new->size) == -1) {
        freePsm(new);
        return NULL;
    }

    int prot = PROT_READ; // default
    if(oflag & O_RDWR) {
        prot = prot | PROT_WRITE;
    }

    new->addr = mmap(NULL, new->size, prot, MAP_SHARED, new->fd, 0);
    if(new->addr == (void *) -1) {
        freePsm(new);
        return NULL;
    }
    return new;
}

void freePsm(psmADT psm) {
    if(psm == NULL) return;
    munmap(psm->addr, psm->size);
    close(psm->fd);
    // CHECK: shm_unlink (only last user should close it)
    // maybe solvabe using a counter in the actual memory space
    shm_unlink(psm->name);
    free(psm);
    return;
}
