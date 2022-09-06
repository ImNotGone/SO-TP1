#ifndef _PSM_ADT_H
#define _PSM_ADT_H

#define _BSD_SOURCE // SOURCE: man ftruncate (docker)

#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_NAME_LEN 256

typedef struct psmCDT * psmADT;

// creates a new posix shared memory space
psmADT newPsm(const char * name, int oflag, mode_t mode);

// allows for writing in the posix shared memory space
size_t writePsm(psmADT psm, const char * buff, size_t bytes);

// allows for reading in the posix shared memory space
size_t readPsm(psmADT psm, char * buff, size_t bytes);

// frees the posix shared memory space
void freePsm(psmADT psm);

#endif
