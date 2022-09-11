#ifndef _PSHM_ADT_H
#define _PSHM_ADT_H

#ifndef _BSD_SOURCE
#define _BSD_SOURCE // SOURCE: man ftruncate (docker)
#endif

#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PSM_SIZE INT_MAX
#define END_IDENTIFIER '\n'

#define MAX_NAME_LEN 256

typedef struct pshmCDT * pshmADT;

// creates a new posix shared memory space
pshmADT newPshm(const char * shmName, int oflag, mode_t mode);

// allows for writing in the posix shared memory space
ssize_t writePshm(pshmADT pshm, const char * buff, size_t bytes);

// allows for reading in the posix shared memory space
ssize_t readPshm(pshmADT pshm, char * buff, size_t bytes);

// frees the posix shared memory space
void freePshm(pshmADT pshm);

#endif
