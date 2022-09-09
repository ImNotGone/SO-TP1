#ifndef _PSM_ADT_H
#define _PSM_ADT_H

#ifndef _BSD_SOURCE
#define _BSD_SOURCE // SOURCE: man ftruncate (docker)
#endif

#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_NAME_LEN 256

typedef struct pshmCDT * pshmADT;

// creates a new posix shared memory space
pshmADT newPshm(const char * shm_name, int oflag, mode_t mode);

// allows for writing in the posix shared memory space
size_t writePshm(pshmADT pshm, const char * buff, size_t bytes);

// allows for reading in the posix shared memory space
size_t readPshm(pshmADT pshm, char * buff, size_t bytes);

// frees the posix shared memory space
void freePshm(pshmADT pshm);

#endif
