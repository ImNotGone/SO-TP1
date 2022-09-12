// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "pshmADT.h"

typedef int fd_t;

typedef struct pshmCDT {
    const char * shmName;
    const char * semRwName;
    sem_t * semRw;
    char * addr;
    int writeOff;
    int readOff;
    fd_t fd;
    size_t size;
} pshmCDT;

#define SEM_RW "/SEM_RW"
#define SHM_ERROR -1

pshmADT newPshm(const char *shmName, int oflag, mode_t mode) {
    if (shmName == NULL) {
        errno = EINVAL; // Invalid argument
        return NULL;
    }
    pshmADT new = malloc(sizeof(pshmCDT));
    if (new == NULL) {
        return NULL;
    }

    new->shmName = shmName;
    new->semRwName = SEM_RW;

    new->fd = shm_open(new->shmName, oflag, mode);
    if (new->fd == SHM_ERROR) {
        free(new);
        return NULL;
    }

    new->writeOff = 0;
    new->readOff = 0;
    new->size = MAX_PSM_SIZE;

    if (oflag & O_CREAT) {
        // If the user wants to create it, use ftruncate
        if (ftruncate(new->fd, new->size * sizeof(char)) == SHM_ERROR) {
            freePshm(new);
            return NULL;
        }
    }

    new->semRw = sem_open(new->semRwName, O_CREAT, mode, 0);
    if (new->semRw == SEM_FAILED) {
        freePshm(new);
        return NULL;
    }

    int prot = PROT_READ; // Default
    if (oflag & O_RDWR) {
        prot = prot | PROT_WRITE; // If write is enabled
    }

    new->addr = mmap(NULL, new->size * sizeof(char), prot, MAP_SHARED, new->fd, 0);
    if (new->addr == (void *) SHM_ERROR) {
        freePshm(new);
        return NULL;
    }

    return new;
}

// Writes a line to the shared memory
ssize_t writePshm(pshmADT pshm, const char *buff, size_t bytes) {
    if (pshm == NULL || bytes == 0) {
        errno = EINVAL;
        return -1;
    }

    // Critical section
    ssize_t bytesWritten;
    for  (bytesWritten = 0; bytesWritten < bytes; bytesWritten++) {

        // Stop writing if string ends
        if (buff[bytesWritten] == '\0') {
            break;
        }

        pshm->addr[pshm->writeOff++] = buff[bytesWritten];

        // Stop writing at end of line
        if (buff[bytesWritten] == END_IDENTIFIER) {
            bytesWritten++;
            break;
        }
    }

    sem_post(pshm->semRw);

    return bytesWritten;
}

// Reads a line from the shared memory
ssize_t readPshm(pshmADT pshm, char *buff, size_t bytes) {
    if (pshm == NULL || bytes == 0) {
        errno = EINVAL;
        return -1;
    }
    sem_wait(pshm->semRw);
    // Critical section
    ssize_t bytesRead;
    // Read byte for byte until bytes is reached, if byte equals END_IDENTIFIER,
    // stop reading
    for (bytesRead = 0; bytesRead < bytes; bytesRead++) {
        buff[bytesRead] = pshm->addr[pshm->readOff++];

        // Stop reading at end of line
        if (pshm->addr[pshm->readOff] == END_IDENTIFIER) {
            bytesRead++;
            break;
        }
    }

    if(bytesRead < bytes) {
        buff[bytesRead] = '\0';
    }

    return bytesRead;
}

void freePshm(pshmADT pshm) {
    if (pshm == NULL) {
        errno = EINVAL;
        return;
    }

    munmap(pshm->addr, pshm->size);
    close(pshm->fd);

    shm_unlink(pshm->shmName);
    sem_unlink(pshm->semRwName);

    sem_close(pshm->semRw);
    free(pshm);
    return;
}
