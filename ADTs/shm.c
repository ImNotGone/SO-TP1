#include "shm.h"
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

typedef int fd_t;

static fd_t get_shm(char * name, size_t size, int oflag, mode_t mode) {
    fd_t fd = shm_open(name, O_RDWR | oflag, PROT_READ|PROT_WRITE);
    if(fd == SHM_ERROR) {
        perror("shm_open");
        return SHM_ERROR;
    }

    if(oflag & O_CREAT) {
        if (ftruncate(fd, size * sizeof(char)) == SHM_ERROR) {
            perror("ftruncate");
            return SHM_ERROR;
        }
    }

    return fd;
}

char * map_shm(char * name, size_t size, bool creator) {
    int flag = O_RDWR;
    if(creator) {
        flag = flag | O_CREAT;
    }

    fd_t fd = get_shm(name, size, flag, PROT_READ|PROT_WRITE);

    if(fd == SHM_ERROR) {
        return NULL;
    }

    char * out = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    if(out == (void*) SHM_ERROR) {
        perror("mmap");
        return NULL;
    }
    return out;
}

ssize_t write_shm(char * addr, char * buff, size_t bytes) {
    strncpy(addr, buff, bytes);
    return 1;
}

ssize_t read_shm(char * addr, char * buff, size_t bytes) {
    snprintf(buff, bytes, addr);
    return 1;
}


bool unmap_shm(char * addr, size_t size) {
    return (munmap(addr, size) != SHM_ERROR);
}

bool destroy_shm(char * name) {
    return (shm_unlink(name) != SHM_ERROR);
}
